#include <agz/tracer/core/bsdf.h>
#include <agz/tracer/core/camera.h>
#include <agz/tracer/core/light.h>
#include <agz/tracer/core/material.h>
#include <agz/tracer/core/medium.h>
#include <agz/tracer/core/sampler.h>
#include <agz/tracer/core/scene.h>
#include <agz/tracer/render/particle_tracing.h>

AGZ_TRACER_RENDER_BEGIN

void trace_particle(
    const ParticleTraceParams &params, const Scene &scene, Sampler &sampler,
    FilmFilterApplier::FilmGridView<Spectrum> &film, Arena &arena)
{
    const auto [light, select_light_pdf] = scene.sample_light(sampler.sample1());
    if(!light)
        return;

    const auto emit_result = light->sample_emit(sampler.sample5());
    if(!emit_result.radiance)
        return;

    const Camera *camera = scene.get_camera();

    Spectrum coef = emit_result.radiance /
        (select_light_pdf * emit_result.pdf_pos * emit_result.pdf_dir);
    coef *= std::abs(cos(emit_result.dir, emit_result.nor));

    Ray r(emit_result.pos + EPS() * emit_result.nor, emit_result.dir);

    for(int depth = 1; depth <= params.max_depth; ++depth)
    {
        if(depth > params.min_depth)
        {
            if(sampler.sample1().u > params.cont_prob)
                return;
            coef /= params.cont_prob;
        }

        // find entity intersection & construct bsdf

        EntityIntersection inct;
        if(!scene.closest_intersection(r, &inct))
            return;
        const ShadingPoint shd = inct.material->shade(inct, arena);

        // sample camera

        const auto camera_sample = camera->sample_wi(
            inct.pos, sampler.sample2());
        if(!camera_sample.we.is_black())
        {
            if(scene.visible(camera_sample.pos_on_cam, inct.pos))
            {
                const Spectrum bsdf_f = shd.bsdf->eval_all(
                    inct.wr, camera_sample.ref_to_pos, TransMode::Radiance);
                if(!bsdf_f.is_black())
                {
                    const real pixel_x = camera_sample.film_coord.x
                                       * params.film_res.x;
                    const real pixel_y = camera_sample.film_coord.y
                                       * params.film_res.y;

                    const real abscos = std::abs(cos(
                        inct.geometry_coord.z, camera_sample.ref_to_pos));
                    const Spectrum f = coef * bsdf_f * abscos
                        * camera_sample.we / camera_sample.pdf;
                    film.apply(pixel_x, pixel_y, f);
                }
            }
        }

        // sample bsdf & construct next ray

        const auto bsdf_sample = shd.bsdf->sample_all(
            inct.wr, TransMode::Importance, sampler.sample3());
        if(!bsdf_sample.f)
            return;

        const real abscos = std::abs(cos(bsdf_sample.dir, inct.geometry_coord.z));
        coef *= bsdf_sample.f * abscos / bsdf_sample.pdf;
        r = Ray(inct.eps_offset(bsdf_sample.dir), bsdf_sample.dir);
    }
}

void trace_vol_particle(
    const ParticleTraceParams &params,
    const Scene &scene, Sampler &sampler,
    FilmFilterApplier::FilmGridView<Spectrum> &film,
    Arena &arena)
{
    const auto [light, select_light_pdf] = scene.sample_light(sampler.sample1());
    if(!light)
        return;

    const auto emit_result = light->sample_emit(sampler.sample5());
    if(!emit_result.radiance)
        return;

    const Camera *camera = scene.get_camera();

    Spectrum coef = emit_result.radiance /
        (select_light_pdf * emit_result.pdf_pos * emit_result.pdf_dir);
    coef *= std::abs(cos(emit_result.dir, emit_result.nor));

    Ray r(emit_result.pos + EPS() * emit_result.nor, emit_result.dir);

    for(int depth = 1; depth <= params.max_depth; ++depth)
    {
        if(depth > params.min_depth)
        {
            if(sampler.sample1().u > params.cont_prob)
                return;
            coef /= params.cont_prob;
        }

        EntityIntersection inct;
        if(!scene.closest_intersection(r, &inct))
            return;

        const Medium *medium = inct.medium(inct.wr);
        const auto med_sam = medium->sample_scattering(
            r.o, inct.pos, sampler, arena);
        coef *= med_sam.throughput;

        // process medium scattering
        if(med_sam.is_scattering_happened())
        {
            const auto &scatter = med_sam.scattering_point;

            // sample camera

            const auto cam_sam = camera->sample_wi(
                scatter.pos, sampler.sample2());
            if(!cam_sam.we.is_black())
            {
                if(scene.visible(cam_sam.pos_on_cam, scatter.pos))
                {
                    const Spectrum bsdf_f = med_sam.phase_function->eval_all(
                        scatter.wr, cam_sam.ref_to_pos, TransMode::Radiance);
                    if(!bsdf_f.is_black())
                    {
                        const real pixel_x = cam_sam.film_coord.x
                            * params.film_res.x;
                        const real pixel_y = cam_sam.film_coord.y
                            * params.film_res.y;

                        const Spectrum tr = medium->tr(
                            cam_sam.pos_on_cam, scatter.pos, sampler);

                        const Spectrum f = coef * bsdf_f * tr
                                         * cam_sam.we / cam_sam.pdf;
                        film.apply(pixel_x, pixel_y, f);
                    }
                }
            }

            // sample phase function

            const auto phase_sample = med_sam.phase_function->sample_all(
                scatter.wr, TransMode::Importance, sampler.sample3());
            if(!phase_sample.f)
                return;

            coef *= phase_sample.f / phase_sample.pdf;
            r = Ray(scatter.pos, phase_sample.dir);

            continue;
        }

        // process surface scattering

        const ShadingPoint shd = inct.material->shade(inct, arena);

        const auto camera_sample = camera->sample_wi(
            inct.pos, sampler.sample2());
        if(!camera_sample.we.is_black())
        {
            if(scene.visible(camera_sample.pos_on_cam, inct.pos))
            {
                const Spectrum bsdf_f = shd.bsdf->eval_all(
                    inct.wr, camera_sample.ref_to_pos, TransMode::Radiance);
                if(!bsdf_f.is_black())
                {
                    const real pixel_x = camera_sample.film_coord.x
                                       * params.film_res.x;
                    const real pixel_y = camera_sample.film_coord.y
                                       * params.film_res.y;

                    const real abscos = std::abs(cos(
                        inct.geometry_coord.z, camera_sample.ref_to_pos));

                    const Spectrum tr = medium->tr(
                        camera_sample.pos_on_cam, inct.pos, sampler);

                    const Spectrum f = coef * bsdf_f * abscos * tr
                                     * camera_sample.we / camera_sample.pdf;
                    film.apply(pixel_x, pixel_y, f);
                }
            }
        }

        // sample bsdf & construct next ray

        const auto bsdf_sample = shd.bsdf->sample_all(
            inct.wr, TransMode::Importance, sampler.sample3());
        if(!bsdf_sample.f)
            return;

        const real abscos = std::abs(cos(bsdf_sample.dir, inct.geometry_coord.z));
        coef *= bsdf_sample.f * abscos / bsdf_sample.pdf;
        r = Ray(inct.eps_offset(bsdf_sample.dir), bsdf_sample.dir);
    }
}

AGZ_TRACER_RENDER_END
