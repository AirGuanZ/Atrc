#include <atomic>
#include <mutex>
#include <string>
#include <thread>

#include <agz/tracer/core/bsdf.h>
#include <agz/tracer/core/camera.h>
#include <agz/tracer/core/entity.h>
#include <agz/tracer/core/light.h>
#include <agz/tracer/core/material.h>
#include <agz/tracer/core/renderer.h>
#include <agz/tracer/core/reporter.h>
#include <agz/tracer/core/sampler.h>
#include <agz/tracer/core/scene.h>
#include <agz/tracer/factory/raw/renderer.h>
#include <agz/utility/thread.h>

AGZ_TRACER_BEGIN

class ParticleTracingRenderer : public Renderer
{
public:

    explicit ParticleTracingRenderer(const ParticleTracingRendererParams &params)
        : params_(params)
    {

    }

    RenderTarget render(const FilmFilterApplier &filter, Scene &scene, ProgressReporter &reporter) override
    {
        scene.start_rendering();
        reporter.begin();

        // backward rendering

        reporter.message("start backward rendering");
        reporter.new_stage();

        Image2D<Spectrum> backward_image = render_backward(filter, scene, reporter);

        reporter.end_stage();

        // forward rendering

        reporter.message("start forward rendering");
        reporter.new_stage();

        RenderTarget render_target = render_forward(filter, scene, reporter);

        reporter.end_stage();

        // merging

        render_target.image = render_target.image + backward_image;

        reporter.end();
        reporter.message("total time: " + std::to_string(reporter.total_seconds()) + "s");

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

    using ForwardGrid = FilmFilterApplier::FilmGrid<Spectrum, real, Spectrum, Vec3, real>;
    using BackwardGrid = FilmFilterApplier::FilmGrid<Spectrum>;

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
                pixel.value = light->radiance(inct.pos, inct.geometry_coord.z, -r.d);
            return pixel;
        }

        if(auto light = scene.envir_light())
            pixel.value += light->radiance(r.o, r.d);
        return pixel;
    }

    void trace_particle(const Scene &scene, BackwardGrid &film_grid, Sampler &sampler, const Vec2 &film_res, Arena &arena) const
    {
        const auto [light, select_light_pdf] = scene.sample_light(sampler.sample1());
        if(!light)
            return;

        const auto emit_result = light->emit(sampler.sample5());
        if(!emit_result.radiance)
            return;

        const Camera *camera = scene.get_camera();

        Spectrum coef = emit_result.radiance / (select_light_pdf * emit_result.pdf_pos * emit_result.pdf_dir);
        coef *= std::abs(cos(emit_result.dir, emit_result.nor));

        Ray r(emit_result.pos + EPS * emit_result.nor, emit_result.dir);

        for(int depth = 1; depth <= params_.max_depth; ++depth)
        {
            if(depth > params_.min_depth)
            {
                if(sampler.sample1().u > params_.cont_prob)
                    return;
                coef /= params_.cont_prob;
            }

            // find entity intersection & construct bsdf

            EntityIntersection inct;
            if(!scene.closest_intersection(r, &inct))
                return;
            const ShadingPoint shd = inct.material->shade(inct, arena);

            // sample camera

            const auto camera_sample = camera->sample_wi(inct.pos, sampler.sample2());
            if(!camera_sample.we.is_black())
            {
                if(scene.visible(camera_sample.pos_on_cam, inct.pos))
                {
                    const Spectrum bsdf_f = shd.bsdf->eval(inct.wr, camera_sample.ref_to_pos, TransportMode::Radiance);
                    if(!bsdf_f.is_black())
                    {
                        const real pixel_x = camera_sample.film_coord.x * film_res.x;
                        const real pixel_y = camera_sample.film_coord.y * film_res.y;

                        const Spectrum f = coef * bsdf_f * std::abs(cos(inct.geometry_coord.z, camera_sample.ref_to_pos))
                                         * camera_sample.we / camera_sample.pdf;
                        film_grid.apply(pixel_x, pixel_y, f);
                    }
                }
            }

            // sample bsdf & construct next ray

            const auto bsdf_sample = shd.bsdf->sample(inct.wr, TransportMode::Importance, sampler.sample3());
            if(!bsdf_sample.f)
                return;

            coef *= bsdf_sample.f * std::abs(cos(bsdf_sample.dir, inct.geometry_coord.z)) / bsdf_sample.pdf;
            r = Ray(inct.eps_offset(bsdf_sample.dir), bsdf_sample.dir);
        }
    }

    RenderTarget render_forward(const FilmFilterApplier &filter, const Scene &scene, ProgressReporter &reporter) const
    {
        const int width = filter.width();
        const int height = filter.height();
        const int x_task_count = (width + params_.forward_task_grid_size - 1) / params_.forward_task_grid_size;
        const int y_task_count = (height + params_.forward_task_grid_size - 1) / params_.forward_task_grid_size;
        const int total_task_count = x_task_count * y_task_count;

        std::atomic<int> next_task_id = 0;
        std::mutex reporter_mutex;

        ImageBufferTemplate<true, true, true, true, true> image_buffer(filter.width(), filter.height());

        auto forward_func = [
            &image_buffer,
            &filter,
            &scene,
            &reporter,
            &reporter_mutex,
            &next_task_id,
            x_task_count,
            total_task_count,
            task_grid_size = params_.forward_task_grid_size,
            this
        ] (Sampler *sampler)
        {
            for(;;)
            {
                const int task_id = next_task_id++;
                if(task_id >= total_task_count)
                    break;

                const int grid_y_index = task_id / x_task_count;
                const int grid_x_index = task_id % x_task_count;
                const int x_beg = grid_x_index * task_grid_size;
                const int y_beg = grid_y_index * task_grid_size;
                const int x_end = (std::min)(x_beg + task_grid_size, filter.width());
                const int y_end = (std::min)(y_beg + task_grid_size, filter.height());

                ForwardGrid film_grid = filter.bind_to(
                    { { x_beg, y_beg }, { x_end - 1, y_end - 1 } },
                    image_buffer.value, image_buffer.weight,
                    image_buffer.albedo, image_buffer.normal, image_buffer.denoise);

                Arena arena;
                const Camera *camera = scene.get_camera();

                const auto sam_pixels = film_grid.sample_pixels();

                for(int py = sam_pixels.low.y; py <= sam_pixels.high.y; ++py)
                {
                    for(int px = sam_pixels.low.x; px <= sam_pixels.high.x; ++px)
                    {
                        sampler->start_pixel(px, py);
                        do
                        {
                            const Sample2 film_sam = sampler->sample2();
                            const real pixel_x = px + film_sam.u;
                            const real pixel_y = py + film_sam.v;
                            const real film_x = pixel_x / filter.width();
                            const real film_y = pixel_y / filter.height();

                            const auto cam_ray = camera->sample_we({ film_x, film_y }, sampler->sample2());

                            const Ray ray(cam_ray.pos_on_cam, cam_ray.pos_to_out);
                            auto pixel = trace_camera_ray(scene, ray, arena);

                            film_grid.apply(
                                pixel_x, pixel_y,
                                cam_ray.throughput * pixel.value, 1,
                                pixel.albedo, pixel.normal, pixel.denoise);

                            arena.release();

                        } while(sampler->next_sample());
                    }
                }

                const real percent = real(100) * (task_id + 1) / total_task_count;
                std::lock_guard lk(reporter_mutex);
                reporter.progress(percent);
            }
        };

        const int worker_count = thread::actual_worker_count(params_.worker_count);
        Arena sampler_arena;

        std::vector<std::thread> threads;
        threads.reserve(worker_count);

        for(int i = 0; i < worker_count; ++i)
        {
            auto sampler = params_.forward_sampler_prototype->clone(i, sampler_arena);
            threads.emplace_back(forward_func, sampler);
        }

        for(auto &t : threads)
            t.join();

        auto ratio = image_buffer.weight.map([](float w)
        {
            return w > 0 ? 1 / w : real(1);
        });

        RenderTarget ret;
        ret.image   = image_buffer.value * ratio;
        ret.albedo  = image_buffer.albedo * ratio;
        ret.normal  = image_buffer.normal * ratio;
        ret.denoise = image_buffer.denoise * ratio;

        return ret;
    }

    Image2D<Spectrum> render_backward(const FilmFilterApplier &filter, const Scene &scene, ProgressReporter &reporter) const
    {
        std::mutex reporter_mutex;
        std::atomic<int> next_task_id = 0;

        const Vec2 film_res_f = {
            static_cast<float>(filter.width()),
            static_cast<float>(filter.height())
        };

        auto backward_func = [
            &filter,
            &scene,
            &film_res_f,
            &reporter,
            &reporter_mutex,
            &next_task_id,
            this
        ] (Sampler *sampler, Image2D<Spectrum> *image)
        {
            auto film_grid = filter.bind_to({ { 0, 0 }, { filter.width(), filter.height() } }, *image);

            for(;;)
            {
                const int task_id = next_task_id++;
                if(task_id >= params_.particle_task_count)
                    break;

                Arena arena;
                sampler->start_pixel(0, task_id);
                do
                {
                    trace_particle(scene, film_grid, *sampler, film_res_f, arena);
                    arena.release();

                } while(sampler->next_sample());

                const real percent = real(100) * (task_id + 1) / params_.particle_task_count;
                std::lock_guard lk(reporter_mutex);
                reporter.progress(percent);
            }
        };

        const int worker_count = thread::actual_worker_count(params_.worker_count);
        Arena sampler_arena;

        std::vector<std::thread> threads;
        std::vector<Image2D<Spectrum>> images;

        threads.reserve(worker_count);
        images.resize(worker_count, Image2D<Spectrum>(filter.height(), filter.width()));

        for(int i = 0; i < worker_count; ++i)
        {
            auto sampler = params_.particle_sampler_prototype->clone(i, sampler_arena);
            threads.emplace_back(backward_func, sampler, &images[i]);
        }

        for(auto &t : threads)
            t.join();

        const int total_particle_count = params_.particle_task_count * params_.particle_sampler_prototype->get_spp();
        const real scale = filter.width() * filter.height() / static_cast<real>(total_particle_count);

        Image2D<Spectrum> ret(filter.height(), filter.width());
        for(int y = 0; y < ret.height(); ++y)
        {
            for(int x = 0; x < ret.width(); ++x)
            {
                for(auto &img : images)
                    ret(y, x) += img(y, x);
                ret(y, x) *= scale;
            }
        }

        return ret;
    }

    ParticleTracingRendererParams params_;
};

std::shared_ptr<Renderer> create_particle_tracing_renderer(
    const ParticleTracingRendererParams &params)
{
    return std::make_shared<ParticleTracingRenderer>(params);
}

AGZ_TRACER_END
