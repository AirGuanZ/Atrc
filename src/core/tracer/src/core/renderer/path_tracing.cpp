#include <atomic>
#include <mutex>
#include <thread>

#include <agz/tracer/core/bsdf.h>
#include <agz/tracer/core/camera.h>
#include <agz/tracer/core/entity.h>
#include <agz/tracer/core/material.h>
#include <agz/tracer/core/medium.h>
#include <agz/tracer/core/renderer.h>
#include <agz/tracer/core/reporter.h>
#include <agz/tracer/core/sampler.h>
#include <agz/tracer/core/scene.h>
#include <agz/tracer/factory/raw/renderer.h>
#include <agz/tracer/utility/direct_illum.h>
#include <agz/utility/thread.h>

AGZ_TRACER_BEGIN

class PathTracingRenderer : public Renderer
{
    using ImageBuffer = ImageBufferTemplate<true, true, true, true, true>;

    struct Pixel
    {
        Spectrum value;
        Spectrum albedo;
        Vec3     normal;
        real     denoise = 1;
    };

    // image value, weight, albedo, normal, d
    using Grid = FilmFilterApplier::FilmGrid<Spectrum, real, Spectrum, Vec3, real>;

    PathTracingRendererParams params_;

    Pixel(PathTracingRenderer::*eval_func_)(const Scene&, const Ray&, Sampler&, Arena&) const;

    Pixel eval_mis(const Scene &scene, const Ray &ray, Sampler &sampler, Arena &arena) const
    {
        Spectrum coef(1);
        Ray r = ray;

        Pixel pixel;
    
        for(int depth = 1; depth <= params_.max_depth; ++depth)
        {
            // apply RR strategy
    
            if(depth > params_.min_depth)
            {
                if(sampler.sample1().u > params_.cont_prob)
                    return pixel;
                coef /= params_.cont_prob;
            }
    
            // find closest entity intersection
    
            EntityIntersection ent_inct;
            const bool has_ent_inct = scene.closest_intersection(r, &ent_inct);
            if(!has_ent_inct)
            {
                if(depth == 1)
                {
                    if(auto light = scene.envir_light())
                        pixel.value += coef * light->radiance(r.o, r.d);
                }
                return pixel;
            }
    
            // fill gbuffer
    
            const ShadingPoint ent_shd = ent_inct.material->shade(ent_inct, arena);
            if(depth == 1)
            {
                pixel.normal = ent_shd.shading_normal;
                pixel.albedo = ent_shd.bsdf->albedo();
                if(ent_inct.entity->get_no_denoise_flag())
                    pixel.denoise = 0;
            }
    
            // sample medium scattering
    
            const auto medium = ent_inct.wr_medium();
            const auto medium_sample = medium->sample_scattering(r.o, ent_inct.pos, sampler, arena);
    
            coef *= medium_sample.throughput;
    
            // process medium scattering
    
            if(medium_sample.is_scattering_happened())
            {
                const auto &scattering_point = medium_sample.scattering_point;
                const auto phase_function = medium_sample.phase_function;
    
                // compute direct illumination
    
                Spectrum direct_illum;
                for(auto light : scene.lights())
                    direct_illum += coef * mis_sample_light(scene, light, scattering_point, phase_function, sampler);
                direct_illum += coef * mis_sample_bsdf(scene, scattering_point, phase_function, sampler);
    
                pixel.value += direct_illum;
    
                // sample phase function
    
                const auto bsdf_sample = phase_function->sample(
                    scattering_point.wr, TransportMode::Radiance, sampler.sample3());
                if(!bsdf_sample.f || bsdf_sample.pdf < EPS)
                    return pixel;
    
                r = Ray(scattering_point.pos, bsdf_sample.dir.normalize());
                coef *= bsdf_sample.f / bsdf_sample.pdf;
                continue;
            }
    
            // process surface scattering
    
            if(depth == 1)
            {
                if(auto light = ent_inct.entity->as_light())
                    pixel.value += coef * light->radiance(ent_inct.pos, ent_inct.geometry_coord.z, ent_inct.wr);
            }
    
            // direct illumination
    
            Spectrum direct_illum;
            for(auto light : scene.lights())
                direct_illum += coef * mis_sample_light(scene, light, ent_inct, ent_shd, sampler);
            direct_illum += coef * mis_sample_bsdf(scene, ent_inct, ent_shd, sampler);
    
            pixel.value += direct_illum;
    
            // sample bsdf
    
            auto bsdf_sample = ent_shd.bsdf->sample(ent_inct.wr, TransportMode::Radiance, sampler.sample3());
            if(!bsdf_sample.f || bsdf_sample.pdf < EPS)
                return pixel;
    
            r = Ray(ent_inct.eps_offset(bsdf_sample.dir), bsdf_sample.dir.normalize());
            coef *= bsdf_sample.f * std::abs(cos(ent_inct.geometry_coord.z, bsdf_sample.dir)) / bsdf_sample.pdf;
        }
    
        return pixel;
    }
    
    Pixel eval_nomis(const Scene &scene, const Ray &ray, Sampler &sampler, Arena &arena) const
    {
        Spectrum coef(1);
        Ray r = ray;

        Pixel pixel;

        for(int depth = 1; depth <= params_.max_depth; ++depth)
        {
            // RR strategy
    
            if(depth > params_.min_depth)
            {
                if(sampler.sample1().u > params_.cont_prob)
                    return pixel;
                coef /= params_.cont_prob;
            }
    
            // find closest entity intersection
            
            EntityIntersection ent_inct;
            const bool has_ent_inct = scene.closest_intersection(r, &ent_inct);
            if(!has_ent_inct)
            {
                if(auto light = scene.envir_light())
                    pixel.value += coef * light->radiance(r.o, r.d);
                return pixel;
            }
    
            // fill gbuffer
    
            const auto ent_shd = ent_inct.material->shade(ent_inct, arena);
            if(depth == 1)
            {
                pixel.normal = ent_shd.shading_normal;
                pixel.albedo = ent_shd.bsdf->albedo();
                if(ent_inct.entity->get_no_denoise_flag())
                    pixel.denoise = 0;
            }
    
            // sample medium scattering
    
            const auto medium = ent_inct.wr_medium();
            const auto medium_sample = medium->sample_scattering(r.o, ent_inct.pos, sampler, arena);
    
            coef *= medium_sample.throughput;
    
            // process medium scattering
    
            if(medium_sample.is_scattering_happened())
            {
                const auto &scattering_point = medium_sample.scattering_point;
                const auto phase_function = medium_sample.phase_function;
    
                const auto phase_sample = phase_function->sample(ent_inct.wr, TransportMode::Radiance, sampler.sample3());
                if(!phase_sample.f)
                    return pixel;
    
                coef *= phase_sample.f / phase_sample.pdf;
                r = Ray(scattering_point.pos, phase_sample.dir);
    
                continue;
            }
    
            // process surface scattering
    
            if(auto light = ent_inct.entity->as_light())
                pixel.value += coef * light->radiance(ent_inct.pos, ent_inct.geometry_coord.z, ent_inct.wr);
    
            const auto bsdf_sample = ent_shd.bsdf->sample(ent_inct.wr, TransportMode::Radiance, sampler.sample3());
            if(!bsdf_sample.f)
                return pixel;
    
            coef *= bsdf_sample.f * std::abs(cos(ent_inct.geometry_coord.z, bsdf_sample.dir)) / bsdf_sample.pdf;
            r = Ray(ent_inct.eps_offset(bsdf_sample.dir), bsdf_sample.dir);
        }
    
        return pixel;
    }

    void render_grid(const Scene &scene, Sampler &sampler, Grid &grid, const Vec2i &full_res) const
    {
        Arena arena;
        const Camera *camera = scene.get_camera();
        auto sam_bound = grid.sample_pixels();

        for(int py = sam_bound.low.y; py <= sam_bound.high.y; ++py)
        {
            for(int px = sam_bound.low.x; px <= sam_bound.high.x; ++px)
            {
                sampler.start_pixel(px, py);
                do
                {
                    const Sample2 film_sam = sampler.sample2();
                    const real pixel_x = px + film_sam.u;
                    const real pixel_y = py + film_sam.v;
                    const real film_x = pixel_x / full_res.x;
                    const real film_y = pixel_y / full_res.y;

                    auto cam_ray = camera->sample_we({ film_x, film_y }, sampler.sample2());

                    const Ray ray(cam_ray.pos_on_cam, cam_ray.pos_to_out);
                    const Pixel pixel = (this->*eval_func_)(scene, ray, sampler, arena);

                    grid.apply(pixel_x, pixel_y, cam_ray.throughput * pixel.value, 1, pixel.albedo, pixel.normal, pixel.denoise);

                    arena.release();

                    if(stop_rendering_)
                        return;

                } while(sampler.next_sample());
            }
        }
    }

    template<bool REPORTER_WITH_PREVIEW>
    RenderTarget render_impl(FilmFilterApplier filter, Scene &scene, ProgressReporter &reporter)
    {
        int width = filter.width();
        int height = filter.height();
        int x_task_count = (width + params_.task_grid_size - 1) / params_.task_grid_size;
        int y_task_count = (height + params_.task_grid_size - 1) / params_.task_grid_size;
        int total_task_count = x_task_count * y_task_count;

        std::atomic<int> next_task_id = 0;
        std::mutex reporter_mutex;

        ImageBuffer image_buffer(width, height);

        auto get_img = std::function([&]()
        {
            auto ratio = image_buffer.weight.map([](real w)
            {
                return w > 0 ? 1 / w : real(1);
            });
            return image_buffer.value * ratio;
        });

        auto func = [
            &filter,
            &scene,
            &reporter,
            &reporter_mutex,
            &next_task_id,
            &get_img,
            x_task_count,
            total_task_count,
            task_grid_size = params_.task_grid_size,
            this
        ] (Sampler *sampler, ImageBuffer *image_buffer)
        {
            const Vec2i full_res = { filter.width(), filter.height() };

            for(;;)
            {
                if(stop_rendering_)
                    return;

                const int task_id = next_task_id++;
                if(task_id >= total_task_count)
                    break;

                const int grid_x_index = task_id % x_task_count;
                const int grid_y_index = task_id / x_task_count;
                const int x_beg = grid_x_index * task_grid_size;
                const int y_beg = grid_y_index * task_grid_size;
                const int x_end = (std::min)(x_beg + task_grid_size, full_res.x);
                const int y_end = (std::min)(y_beg + task_grid_size, full_res.y);

                auto grid = filter.create_subgrid<Spectrum, real, Spectrum, Vec3, real>(
                    { { x_beg, y_beg }, { x_end - 1, y_end - 1 } });
                this->render_grid(scene, *sampler, grid, full_res);

                if constexpr(REPORTER_WITH_PREVIEW)
                {
                    const real percent = real(100) * (task_id + 1) / total_task_count;
                    std::lock_guard lk(reporter_mutex);
                    grid.merge_into(
                        image_buffer->value, image_buffer->weight,
                        image_buffer->albedo, image_buffer->normal, image_buffer->denoise);
                    reporter.progress(percent, get_img);
                }
                else
                {
                    grid.merge_into(
                        image_buffer->value, image_buffer->weight,
                        image_buffer->albedo, image_buffer->normal, image_buffer->denoise);

                    const real percent = real(100) * (task_id + 1) / total_task_count;
                    std::lock_guard lk(reporter_mutex);
                    reporter.progress(percent, {});
                }
            }
        };

        const int worker_count = thread::actual_worker_count(params_.worker_count);
        std::vector<std::thread> threads;

        Arena sampler_arena;

        reporter.begin();
        reporter.new_stage();

        for(int i = 0; i < worker_count; ++i)
        {
            auto sampler = params_.sampler_prototype->clone(i, sampler_arena);
            threads.emplace_back(func, sampler, &image_buffer);
        }

        for(auto &t : threads)
            t.join();

        reporter.end_stage();
        reporter.end();

        auto ratio = image_buffer.weight.map([](real w)
        {
            return w > 0 ? 1 / w : real(1);
        });

        RenderTarget render_target;
        render_target.image = image_buffer.value * ratio;
        render_target.albedo = image_buffer.albedo * ratio;
        render_target.normal = image_buffer.normal * ratio;
        render_target.denoise = image_buffer.denoise * ratio;

        reporter.message("total time: " + std::to_string(reporter.total_seconds()) + "s");

        return render_target;
    }

public:

    explicit PathTracingRenderer(const PathTracingRendererParams &params)
        : params_(params)
    {
        if(params.use_mis)
            eval_func_ = &PathTracingRenderer::eval_mis;
        else
            eval_func_ = &PathTracingRenderer::eval_nomis;
    }

    RenderTarget render(FilmFilterApplier filter, Scene &scene, ProgressReporter &reporter) override
    {
        if(reporter.need_image_preview())
            return render_impl<true>(filter, scene, reporter);
        return render_impl<false>(filter, scene, reporter);
    }
};

std::shared_ptr<Renderer> create_path_tracing_renderer(const PathTracingRendererParams &params)
{
    return std::make_shared<PathTracingRenderer>(params);
}

AGZ_TRACER_END
