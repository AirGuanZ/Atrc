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

    void render(Scene &scene, ProgressReporter &reporter, Film *film) override
    {
        scene.start_rendering();

        reporter.begin();

        render_backward(scene, reporter, film);

        render_forward(scene, reporter, film);

        int total_particle_count = params_.particle_task_count * params_.particle_sampler_prototype->get_spp();
        film->set_scale(film->resolution().product() / real(total_particle_count));

        reporter.end();

        reporter.message("total time: " + std::to_string(reporter.total_seconds()) + "s");
    }

private:

    void trace_particle(const Scene &scene, FilmGrid &film_grid, Sampler &sampler, const Vec2 &film_res, Arena &arena)
    {
        auto [light, select_light_pdf] = scene.sample_light(sampler.sample1());
        if(!light)
            return;

        auto emit_result = light->emit(sampler.sample5());
        if(!emit_result.radiance)
            return;

        const Camera *camera = scene.camera();

        Spectrum coef = emit_result.radiance / (select_light_pdf * emit_result.pdf_pos * emit_result.pdf_dir);
        if(!emit_result.normal.is_zero())
            coef *= std::abs(cos(emit_result.direction, emit_result.normal));

        Ray r(emit_result.position + EPS * emit_result.normal, emit_result.direction);

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
            ShadingPoint shd = inct.material->shade(inct, arena);

            // sample camera

            auto camera_sample = camera->sample_wi(inct.pos, sampler.sample2());
            if(!camera_sample.we.is_black())
            {
                if(scene.visible(camera_sample.pos_on_cam, inct.pos))
                {
                    Spectrum bsdf_f = shd.bsdf->eval(inct.wr, camera_sample.ref_to_pos, TM_Importance);
                    if(!bsdf_f.is_black())
                    {
                        real pixel_x = camera_sample.film_coord.x * film_res.x;
                        real pixel_y = camera_sample.film_coord.y * film_res.y;

                        Spectrum f = coef * std::abs(cos(camera_sample.nor_at_pos, camera_sample.ref_to_pos))
                                   * bsdf_f * std::abs(cos(inct.geometry_coord.z, camera_sample.ref_to_pos))
                                   * camera_sample.we / camera_sample.pdf;

                        film_grid.add_sample({ pixel_x, pixel_y }, f, GBufferPixel{});
                    }
                }
            }

            // sample bsdf & construct next ray

            auto bsdf_sample = shd.bsdf->sample(inct.wr, TM_Importance, sampler.sample3());
            if(!bsdf_sample.f)
                return;

            coef *= bsdf_sample.f * std::abs(cos(bsdf_sample.dir, inct.geometry_coord.z)) / bsdf_sample.pdf;
            r = Ray(inct.eps_offset(bsdf_sample.dir), bsdf_sample.dir);
        }
    }

    Spectrum trace_camera_ray(const Scene &scene, const Ray &r, GBufferPixel &gpixel, Arena &arena) const
    {
        EntityIntersection inct;
        if(scene.closest_intersection(r, &inct))
        {
            auto shd = inct.material->shade(inct, arena);
            gpixel.position = inct.pos;
            gpixel.depth    = distance(inct.pos, r.o);
            gpixel.normal   = shd.shading_normal;
            gpixel.albedo   = shd.bsdf->albedo();
            gpixel.denoise  = inct.entity->get_no_denoise_flag() ? real(0) : real(1);

            if(auto light = inct.entity->as_light())
                return light->radiance(inct.pos, inct.geometry_coord.z, -r.d);
            return { };
        }

        Spectrum ret;
        for(auto light : scene.nonarea_lights())
            ret += light->radiance(r.o, r.d);
        return ret;
    }

    void exec_particle_tracing_task(const Scene &scene, int task_id, FilmGrid &film_grid, Sampler &sampler, const Vec2 &film_res)
    {
        Arena arena;
        sampler.start_pixel(0, task_id);
        do
        {
            trace_particle(scene, film_grid, sampler, film_res, arena);
            arena.release();

        } while(sampler.next_sample());
    }

    void exec_forward_tracing_task(const Scene &scene, FilmGrid *film_grid, real value_scale, Sampler &sampler, const Vec2i &full_res)
    {
        Arena arena;
        const Camera *camera = scene.camera();
        int x_beg = film_grid->sample_x_beg();
        int x_end = film_grid->sample_x_end();
        int y_beg = film_grid->sample_y_beg();
        int y_end = film_grid->sample_y_end();

        real scale = value_scale / sampler.get_spp();

        for(int py = y_beg; py < y_end; ++py)
        {
            for(int px = x_beg; px < x_end; ++px)
            {
                sampler.start_pixel(px, py);
                do
                {
                    Sample2 film_sam = sampler.sample2();
                    real pixel_x = px + film_sam.u;
                    real pixel_y = py + film_sam.v;
                    real film_x = pixel_x / full_res.x;
                    real film_y = pixel_y / full_res.y;

                    auto cam_ray = camera->sample_we({ film_x, film_y }, sampler.sample2());

                    GBufferPixel gpixel;
                    Ray ray(cam_ray.pos_on_cam, cam_ray.pos_to_out);
                    Spectrum value = trace_camera_ray(scene, ray, gpixel, arena);

                    value *= cam_ray.throughput * scale;
                    gpixel.position *= scale;
                    gpixel.depth    *= scale;
                    gpixel.normal   *= scale;
                    gpixel.albedo   *= scale;
                    gpixel.denoise  *= scale;

                    film_grid->add_sample({ pixel_x, pixel_y }, value, gpixel, 1);

                    arena.release();

                } while(sampler.next_sample());
            }
        }
    }

    void render_backward(Scene &scene, ProgressReporter &reporter, Film *film)
    {
        std::mutex reporter_mutex;
        std::atomic<int> next_particle_task_id = 0;

        Vec2 film_res_f = {
            static_cast<real>(film->resolution().x),
            static_cast<real>(film->resolution().y)
        };

        auto backward_func = [
            &scene,
            &film_res_f,
            &reporter,
            &reporter_mutex,
            &next_particle_task_id,
            this
        ] (Sampler *sampler, FilmGrid *film_grid)
        {
            for(;;)
            {
                int task_id = next_particle_task_id++;
                if(task_id >= params_.particle_task_count)
                    break;

                exec_particle_tracing_task(scene, task_id, *film_grid, *sampler, film_res_f);

                real percent = real(100) * (task_id + 1) / params_.particle_task_count;
                std::lock_guard lk(reporter_mutex);
                reporter.progress(percent);
            }
        };

        int actual_thread_count = thread::actual_worker_count(params_.worker_count);
        Arena sampler_arena;

        std::vector<std::thread> threads;
        std::vector<Sampler*> samplers;
        std::vector<std::unique_ptr<FilmGrid>> film_grids;

        threads.reserve(actual_thread_count);
        samplers.reserve(actual_thread_count);
        film_grids.reserve(actual_thread_count);

        auto film_res_i = film->resolution();

        for(int i = 0; i < actual_thread_count; ++i)
        {
            samplers.push_back(params_.particle_sampler_prototype->clone(i, sampler_arena));
            film_grids.push_back(film->new_grid(0, film_res_i.x, 0, film_res_i.y));
        }

        reporter.message("start backward tracing");
        reporter.new_stage();

        for(int i = 0; i < actual_thread_count; ++i)
            threads.emplace_back(backward_func, samplers[i], film_grids[i].get());

        for(auto &t : threads)
            t.join();

        for(auto &film_grid : film_grids)
            film->merge_grid(*film_grid);

        reporter.end_stage();
    }

    void render_forward(Scene &scene, ProgressReporter &reporter, Film *film)
    {
        auto [film_width, film_height] = film->resolution();
        int x_task_count = (film_width  + params_.forward_task_grid_size - 1) / params_.forward_task_grid_size;
        int y_task_count = (film_height + params_.forward_task_grid_size - 1) / params_.forward_task_grid_size;
        int total_task_count = x_task_count * y_task_count;

        int total_particle_count = params_.particle_task_count * params_.particle_sampler_prototype->get_spp();
        real final_scale = real(film_width * film_height) / total_particle_count;

        std::atomic<int> next_task_id = 0;
        std::mutex reporter_mutex;

        auto forward_func = [
            film,
            &scene,
            &reporter,
            &reporter_mutex,
            &next_task_id,
            x_task_count,
            total_task_count,
            value_scale = 1 / final_scale,
            task_grid_size = params_.forward_task_grid_size,
            this
        ] (Sampler *sampler)
        {
            Vec2i full_res = film->resolution();

            for(;;)
            {
                int this_task_id = next_task_id++;
                if(this_task_id >= total_task_count)
                    return;

                int grid_y_index = this_task_id / x_task_count;
                int grid_x_index = this_task_id % x_task_count;

                int x_beg = grid_x_index * task_grid_size;
                int y_beg = grid_y_index * task_grid_size;
                int x_end = (std::min)(x_beg + task_grid_size, full_res.x);
                int y_end = (std::min)(y_beg + task_grid_size, full_res.y);

                auto film_grid = film->new_grid(x_beg, x_end, y_beg, y_end);
                this->exec_forward_tracing_task(scene, film_grid.get(), value_scale, *sampler, full_res);

                film->merge_grid(*film_grid);

                real percent = real(100) * (this_task_id + 1) / total_task_count;
                std::lock_guard lk(reporter_mutex);
                reporter.progress(percent);
            }
        };

        int actual_thread_count = thread::actual_worker_count(params_.worker_count);
        Arena sampler_arena;

        std::vector<std::thread> threads;
        threads.reserve(actual_thread_count);

        reporter.message("start forward tracing");
        reporter.new_stage();

        for(int i = 0; i < actual_thread_count; ++i)
        {
            auto sampler = params_.forward_sampler_prototype->clone(i, sampler_arena);
            threads.emplace_back(forward_func, sampler);
        }

        for(auto &t : threads)
            t.join();

        reporter.end_stage();
    }

    const ParticleTracingRendererParams params_;
};

std::shared_ptr<Renderer> create_particle_tracing_renderer(
    const ParticleTracingRendererParams &params)
{
    return std::make_shared<ParticleTracingRenderer>(params);
}

AGZ_TRACER_END
