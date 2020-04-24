#include <atomic>
#include <mutex>
#include <thread>

#include <agz/tracer/core/bsdf.h>
#include <agz/tracer/core/camera.h>
#include <agz/tracer/core/entity.h>
#include <agz/tracer/core/light.h>
#include <agz/tracer/core/material.h>
#include <agz/tracer/core/renderer.h>
#include <agz/tracer/core/renderer_interactor.h>
#include <agz/tracer/core/sampler.h>
#include <agz/tracer/core/scene.h>
#include <agz/tracer/create/renderer.h>
#include <agz/tracer/render/particle_tracing.h>
#include <agz/tracer/utility/parallel_grid.h>
#include <agz/utility/thread.h>

AGZ_TRACER_BEGIN

class ParticleTracingRenderer : public Renderer
{
public:

    explicit ParticleTracingRenderer(
        const AdjointPTRendererParams &params)
        : params_(params)
    {
        particle_params_.min_depth = params.min_depth;
        particle_params_.max_depth = params.max_depth;
        particle_params_.cont_prob = params.cont_prob;
    }

    RenderTarget render(
        FilmFilterApplier filter, Scene &scene,
        RendererInteractor &reporter) override
    {
        reporter.begin();

        // backward rendering

        reporter.message("start backward rendering");
        reporter.new_stage();

        particle_params_.film_res =
        {
            real(filter.width()),
            real(filter.height())
        };
        Image2D<Spectrum> backward_image = reporter.need_image_preview() ?
            render_backward<true>(filter, scene, reporter) :
            render_backward<false>(filter, scene, reporter);

        reporter.end_stage();

        // forward rendering

        reporter.message("start forward rendering");
        reporter.new_stage();

        RenderTarget render_target = reporter.need_image_preview() ?
            render_forward<true>(filter, scene, reporter, backward_image) :
            render_forward<false>(filter, scene, reporter, backward_image);

        reporter.end_stage();

        // merging

        render_target.image = render_target.image + backward_image;

        reporter.end();

        return render_target;
    }

private:

    struct Pixel
    {
        Spectrum value;
        Spectrum albedo;
        Vec3     normal;
        real     denoise = 1;
    };

    using ForwardGrid  = FilmFilterApplier::FilmGrid<
                            Spectrum, real, Spectrum, Vec3, real>;
    using BackwardGrid = FilmFilterApplier::FilmGridView<Spectrum>;

    Pixel trace_camera_ray(const Scene &scene, const Ray &r, Arena &arena) const
    {
        Pixel pixel;

        EntityIntersection inct;
        if(scene.closest_intersection(r, &inct))
        {
            auto shd = inct.material->shade(inct, arena);
            pixel.normal  = shd.shading_normal;
            pixel.albedo  = shd.bsdf->albedo();
            pixel.denoise = inct.entity->get_no_denoise_flag() ? real(0) : real(1);

            if(auto light = inct.entity->as_light())
            {
                pixel.value = light->radiance(
                    inct.pos, inct.geometry_coord.z, inct.uv, -r.d);
            }
            return pixel;
        }

        if(auto light = scene.envir_light())
            pixel.value += light->radiance(r.o, r.d);
        return pixel;
    }

    template<bool REPORTER_WITH_PREVIEW>
    RenderTarget render_forward(
        const FilmFilterApplier &filter, const Scene &scene,
        RendererInteractor &reporter, const Image2D<Spectrum> &backward) const
    {
        const int thread_count = thread::actual_worker_count(
            params_.worker_count);

        auto sampler_prototype = newRC<NativeSampler >(42, false);
        Arena sampler_arena;

        std::vector<Sampler *> perthread_sampler;
        for(int i = 0; i < thread_count; ++i)
            perthread_sampler.push_back(sampler_prototype->clone(i, sampler_arena));

        std::mutex reporter_mutex;
        int finished_pixel_count = 0;

        ImageBufferTemplate<true, true, true, true, true> image_buffer(
            filter.width(), filter.height());

        parallel_for_2d_grid(
            thread_count, filter.width(), filter.height(),
            params_.forward_task_grid_size, params_.forward_task_grid_size,
            [&](int thread_index, const Rect2i &rect)
        {
            auto sampler = perthread_sampler[thread_index];

            ForwardGrid film_grid = filter.create_subgrid<
                Spectrum, real, Spectrum, Vec3, real>(
                    { rect.low, rect.high - Vec2i(1) });

            Arena arena;
            const Camera *camera = scene.get_camera();

            const auto sam_pixels = film_grid.sample_pixels();

            for(int py = sam_pixels.low.y; py <= sam_pixels.high.y; ++py)
            {
                for(int px = sam_pixels.low.x; px <= sam_pixels.high.x; ++px)
                {
                    for(int i = 0; i < params_.forward_spp; ++i)
                    {
                        const Sample2 film_sam = sampler->sample2();
                        const real pixel_x = px + film_sam.u;
                        const real pixel_y = py + film_sam.v;
                        const real film_x = pixel_x / filter.width();
                        const real film_y = pixel_y / filter.height();

                        const auto cam_ray = camera->sample_we(
                            { film_x, film_y }, sampler->sample2());

                        const Ray ray(cam_ray.pos_on_cam, cam_ray.pos_to_out);
                        auto pixel = trace_camera_ray(scene, ray, arena);

                        film_grid.apply(
                            pixel_x, pixel_y,
                            cam_ray.throughput * pixel.value, 1,
                            pixel.albedo, pixel.normal, pixel.denoise);

                        arena.release();

                        if(stop_rendering_)
                            return false;
                    }
                }
            }

            if constexpr(REPORTER_WITH_PREVIEW)
            {
                std::lock_guard lk(reporter_mutex);
                film_grid.merge_into(
                    image_buffer.value, image_buffer.weight,
                    image_buffer.albedo, image_buffer.normal,
                    image_buffer.denoise);

                auto get_img = [&]()
                {
                    auto ratio = image_buffer.weight.map([](real w)
                    {
                        return w > 0 ? 1 / w : real(1);
                    });
                    return image_buffer.value * ratio + backward;
                };

                finished_pixel_count += (rect.high - rect.low).product();
                const real percent = real(100) * finished_pixel_count
                                   / real(filter.width() * filter.height());
                reporter.progress(percent, get_img);
            }
            else
            {
                film_grid.merge_into(
                    image_buffer.value, image_buffer.weight,
                    image_buffer.albedo, image_buffer.normal,
                    image_buffer.denoise);

                std::lock_guard lk(reporter_mutex);
                
                finished_pixel_count += (rect.high - rect.low).product();
                const real percent = real(100) * finished_pixel_count
                                   / real(filter.width() * filter.height());
                reporter.progress(percent, {});
            }

            return true;
        });

        auto ratio = image_buffer.weight.map([](float w)
        {
            return w > 0 ? 1 / w : real(1);
        });

        RenderTarget ret;
        ret.image   = image_buffer.value   * ratio;
        ret.albedo  = image_buffer.albedo  * ratio;
        ret.normal  = image_buffer.normal  * ratio;
        ret.denoise = image_buffer.denoise * ratio;

        return ret;
    }

    template<bool REPORTER_WITH_PREVIEW>
    Image2D<Spectrum> render_backward(
        const FilmFilterApplier &filter, const Scene &scene,
        RendererInteractor &reporter) const
    {
        std::mutex reporter_mutex;
        std::atomic<int> next_task_id = 0;

        const int worker_count = thread::actual_worker_count(
            params_.worker_count);

        std::atomic<uint64_t> total_particle_count = 0;

        Box<std::mutex[]> output_img_mutex;
        std::vector<Image2D<Spectrum>> output_img;

        auto backward_func =
            [&](Sampler *sampler, Image2D<Spectrum> *image, int i)
        {
            auto film_grid = filter.create_subgrid_view(
                { { 0, 0 }, { filter.width() - 1, filter.height() - 1 } },
                *image);

            for(;;)
            {
                if(stop_rendering_)
                    return;

                const int task_id = next_task_id++;
                if(task_id >= params_.particle_task_count)
                    break;

                Arena arena;
                int task_particle_count = 0;
                for(int j = 0; j < params_.particles_per_task; ++j)
                {
                    ++task_particle_count;
                    trace_vol_particle(
                        particle_params_, scene, *sampler, film_grid, arena);
                    arena.release();

                    if(stop_rendering_)
                        return;
                }

                if constexpr(REPORTER_WITH_PREVIEW)
                {
                    uint64_t pc;

                    {
                        std::lock_guard lk(output_img_mutex[i]);
                        output_img[i] = *image;
                        total_particle_count += task_particle_count;
                        pc = total_particle_count;
                    }
                    
                    auto get_img = [
                        worker_count, &output_img, &output_img_mutex,
                        &filter, pc]()
                    {
                        Image2D<Spectrum> ret(filter.height(), filter.width());
                        for(int j = 0; j < worker_count; ++j)
                        {
                            std::lock_guard lk(output_img_mutex[j]);
                            ret = ret + output_img[j];
                        }

                        real ratio = filter.width() * filter.height()
                                   * (pc ? real(1) / pc : real(0));
                        return ratio * ret;
                    };

                    const real percent = real(100) * (task_id + 1)
                                       / params_.particle_task_count;
                    std::lock_guard lk(reporter_mutex);
                    reporter.progress(percent, get_img);
                }
                else
                {
                    total_particle_count += task_particle_count;

                    const real percent = real(100) * (task_id + 1)
                                       / params_.particle_task_count;
                    std::lock_guard lk(reporter_mutex);
                    reporter.progress(percent, {});
                }
            }
        };

        Arena sampler_arena;

        std::vector<std::thread> threads;
        std::vector<Image2D<Spectrum>> images;

        threads.reserve(worker_count);
        images.resize(worker_count, Image2D<Spectrum>(
            filter.height(), filter.width()));

        if constexpr(REPORTER_WITH_PREVIEW)
        {
            output_img_mutex = newBox<std::mutex[]>(worker_count);
            output_img.resize(worker_count, Image2D<Spectrum>(
                filter.height(), filter.width()));
        }

        auto particle_sampler_prototype = newRC<NativeSampler>(42, false);

        for(int i = 0; i < worker_count; ++i)
        {
            auto sampler = particle_sampler_prototype->clone(i, sampler_arena);
            threads.emplace_back(backward_func, sampler, &images[i], i);
        }

        for(auto &t : threads)
            t.join();

        const real scale = filter.width() * filter.height()
                         / static_cast<real>(total_particle_count);

        Image2D<Spectrum> ret(filter.height(), filter.width());
        for(auto &img : images)
            ret = ret + img;
        ret = ret * scale;

        return ret;
    }

    AdjointPTRendererParams params_;
    render::ParticleTraceParams particle_params_;
};

RC<Renderer> create_adjoint_pt_renderer(
    const AdjointPTRendererParams &params)
{
    return newRC<ParticleTracingRenderer>(params);
}

AGZ_TRACER_END
