#include <agz/editor/renderer/path_tracer.h>
#include <agz/tracer/core/bsdf.h>
#include <agz/tracer/core/camera.h>
#include <agz/tracer/core/entity.h>
#include <agz/tracer/core/light.h>
#include <agz/tracer/core/material.h>
#include <agz/tracer/core/medium.h>
#include <agz/tracer/core/scene.h>
#include <agz/tracer/factory/raw/sampler.h>
#include <agz/tracer/utility/direct_illum.h>
#include <agz/utility/thread.h>

AGZ_EDITOR_BEGIN

PathTracer::PathTracer(
    const PathTracingParams &params,
    int fb_width, int fb_height,
    std::shared_ptr<const tracer::Scene> scene)
    : stop_rendering_(false), params_(params), scene_(std::move(scene)),
      framebuffer_(fb_width, fb_height, 32)
{
    auto render_func = [this](tracer::Sampler *sampler)
    {
        std::vector<Framebuffer::Task> tasks;
        for(;;)
        {
            if(stop_rendering_)
                return;

            tasks.clear();
            int task_count = framebuffer_.get_tasks(1, tasks);

            for(int i = 0; i < task_count; ++i)
                exec_render_task(tasks[i], sampler);

            framebuffer_.merge_tasks(task_count, tasks.data());
        }
    };

    auto sampler_prototype = tracer::create_native_sampler(1, 0, true);

    int worker_count = thread::actual_worker_count(params_.worker_count);
    for(int i = 0; i < worker_count; ++i)
    {
        auto sampler = sampler_prototype->clone(i, sampler_arena_);
        threads_.emplace_back(render_func, sampler);
    }
}

PathTracer::~PathTracer()
{
    stop_rendering_ = true;
    for(auto &t : threads_)
        t.join();
}

Image2D<math::color3b> PathTracer::get_image() const
{
    return framebuffer_.get_image();
}

Spectrum PathTracer::eval(const tracer::Scene &scene, const tracer::Ray &ray, tracer::Sampler &sampler, tracer::Arena &arena) const
{
    using namespace tracer;

    Spectrum ret, coef(1);
    Ray r = ray;

    for(int depth = 1; depth <= params_.max_depth; ++depth)
    {
        // apply RR strategy

        if(depth > params_.min_depth)
        {
            if(sampler.sample1().u > params_.cont_prob)
                return ret;
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
                    ret += coef * light->radiance(r.o, r.d);
            }
            return ret;
        }

        const ShadingPoint ent_shd = ent_inct.material->shade(ent_inct, arena);

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

            ret += direct_illum;

            // sample phase function

            const auto bsdf_sample = phase_function->sample(
                scattering_point.wr, TransportMode::Radiance, sampler.sample3());
            if(!bsdf_sample.f || bsdf_sample.pdf < EPS)
                return ret;

            r = Ray(scattering_point.pos, bsdf_sample.dir.normalize());
            coef *= bsdf_sample.f / bsdf_sample.pdf;
            continue;
        }

        // process surface scattering

        if(depth == 1)
        {
            if(auto light = ent_inct.entity->as_light())
                ret += coef * light->radiance(ent_inct.pos, ent_inct.geometry_coord.z, ent_inct.wr);
        }

        // direct illumination

        Spectrum direct_illum;
        for(auto light : scene.lights())
            direct_illum += coef * mis_sample_light(scene, light, ent_inct, ent_shd, sampler);
        direct_illum += coef * mis_sample_bsdf(scene, ent_inct, ent_shd, sampler);

        ret += direct_illum;

        // sample bsdf

        auto bsdf_sample = ent_shd.bsdf->sample(ent_inct.wr, TransportMode::Radiance, sampler.sample3());
        if(!bsdf_sample.f || bsdf_sample.pdf < EPS)
            return ret;

        r = Ray(ent_inct.eps_offset(bsdf_sample.dir), bsdf_sample.dir.normalize());
        coef *= bsdf_sample.f * std::abs(cos(ent_inct.geometry_coord.z, bsdf_sample.dir)) / bsdf_sample.pdf;
    }

    return ret;
}

void PathTracer::exec_render_task(Framebuffer::Task &task, tracer::Sampler *sampler)
{
    using namespace tracer;

    Arena arena;
    const Camera *camera = scene_->get_camera();
    const Rect2i sam_bound = task.pixel_range;

    for(int py = sam_bound.low.y; py <= sam_bound.high.y; ++py)
    {
        for(int px = sam_bound.low.x; px <= sam_bound.high.x; ++px)
        {
            sampler->start_pixel(px, py);

            for(int s = 0; s < task.spp; ++s)
            {
                if(stop_rendering_)
                    return;

                const Sample2 film_sam = sampler->sample2();
                const real pixel_x = px + film_sam.u;
                const real pixel_y = py + film_sam.v;
                const real film_x = pixel_x / task.full_res.x;
                const real film_y = pixel_y / task.full_res.y;

                auto cam_ray = camera->sample_we({ film_x, film_y }, sampler->sample2());

                const Ray ray(cam_ray.pos_on_cam, cam_ray.pos_to_out);
                const Spectrum radiance = eval(*scene_, ray, *sampler, arena);

                if(radiance.is_finite())
                {
                    const int lx = px - sam_bound.low.x;
                    const int ly = py - sam_bound.low.y;
                    task.value(ly, lx) += radiance;
                    task.weight(ly, lx) += 1;
                }

            }
        }
    }
}

AGZ_EDITOR_END
