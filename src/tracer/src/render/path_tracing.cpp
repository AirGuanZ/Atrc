#include <agz/tracer/core/bsdf.h>
#include <agz/tracer/core/camera.h>
#include <agz/tracer/core/entity.h>
#include <agz/tracer/core/material.h>
#include <agz/tracer/core/medium.h>
#include <agz/tracer/core/renderer.h>
#include <agz/tracer/core/sampler.h>
#include <agz/tracer/core/scene.h>
#include <agz/tracer/render/direct_illum.h>
#include <agz/tracer/render/path_tracing.h>

AGZ_TRACER_RENDER_BEGIN

Pixel trace_std(const TraceParams &params, const Scene &scene, const Ray &ray, Sampler &sampler, Arena &arena)
{
    Spectrum coef(1);
    Ray r = ray;

    Pixel pixel;

    int scattering_count = 0;

    for(int depth = 1; depth <= params.max_depth; ++depth)
    {
        // apply RR strategy

        if(depth > params.min_depth)
        {
            if(sampler.sample1().u > params.cont_prob)
                return pixel;
            coef /= params.cont_prob;
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

        if(scattering_count < medium->get_max_scattering_count())
        {
            const auto medium_sample = medium->sample_scattering(r.o, ent_inct.pos, sampler, arena);
            coef *= medium_sample.throughput;

            // process medium scattering

            if(medium_sample.is_scattering_happened())
            {
                ++scattering_count;

                const auto &scattering_point = medium_sample.scattering_point;
                const auto phase_function = medium_sample.phase_function;

                // compute direct illumination

                Spectrum direct_illum;
                for(int i = 0; i < params.direct_illum_sample_count; ++i)
                {
                    for(auto light : scene.lights())
                        direct_illum += coef * mis_sample_light(scene, light, scattering_point, phase_function, sampler);
                    direct_illum += coef * mis_sample_bsdf(scene, scattering_point, phase_function, sampler);
                }

                pixel.value += real(1) / params.direct_illum_sample_count * direct_illum;

                // sample phase function

                const auto bsdf_sample = phase_function->sample(
                    scattering_point.wr, TransportMode::Radiance, sampler.sample3());
                if(!bsdf_sample.f || bsdf_sample.pdf < EPS)
                    return pixel;

                r = Ray(scattering_point.pos, bsdf_sample.dir.normalize());
                coef *= bsdf_sample.f / bsdf_sample.pdf;
                continue;
            }
        }

        scattering_count = 0;

        // process surface scattering

        if(depth == 1)
        {
            if(auto light = ent_inct.entity->as_light())
                pixel.value += coef * light->radiance(ent_inct.pos, ent_inct.geometry_coord.z, ent_inct.uv, ent_inct.wr);
        }

        // direct illumination

        Spectrum direct_illum;
        for(int i = 0; i < params.direct_illum_sample_count; ++i)
        {
            for(auto light : scene.lights())
                direct_illum += coef * mis_sample_light(scene, light, ent_inct, ent_shd, sampler);
            direct_illum += coef * mis_sample_bsdf(scene, ent_inct, ent_shd, sampler);
        }

        pixel.value += real(1) / params.direct_illum_sample_count * direct_illum;

        // sample bsdf

        auto bsdf_sample = ent_shd.bsdf->sample(ent_inct.wr, TransportMode::Radiance, sampler.sample3());
        if(!bsdf_sample.f || bsdf_sample.pdf < EPS)
            return pixel;

        r = Ray(ent_inct.eps_offset(bsdf_sample.dir), bsdf_sample.dir.normalize());
        coef *= bsdf_sample.f * std::abs(cos(ent_inct.geometry_coord.z, bsdf_sample.dir)) / bsdf_sample.pdf;
    }

    return pixel;
}

Pixel trace_nomis(const TraceParams &params, const Scene &scene, const Ray &ray, Sampler &sampler, Arena &arena)
{
    Spectrum coef(1);
    Ray r = ray;

    Pixel pixel;

    int scattering_count = 0;

    for(int depth = 1; depth <= params.max_depth; ++depth)
    {
        // RR strategy

        if(depth > params.min_depth)
        {
            if(sampler.sample1().u > params.cont_prob)
                return pixel;
            coef /= params.cont_prob;
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

        if(scattering_count < medium->get_max_scattering_count())
        {
            const auto medium_sample = medium->sample_scattering(r.o, ent_inct.pos, sampler, arena);
            coef *= medium_sample.throughput;

            // process medium scattering

            if(medium_sample.is_scattering_happened())
            {
                ++scattering_count;

                const auto &scattering_point = medium_sample.scattering_point;
                const auto phase_function = medium_sample.phase_function;

                const auto phase_sample = phase_function->sample(ent_inct.wr, TransportMode::Radiance, sampler.sample3());
                if(!phase_sample.f)
                    return pixel;

                coef *= phase_sample.f / phase_sample.pdf;
                r = Ray(scattering_point.pos, phase_sample.dir);

                continue;
            }
        }

        scattering_count = 0;

        // process surface scattering

        if(auto light = ent_inct.entity->as_light())
            pixel.value += coef * light->radiance(ent_inct.pos, ent_inct.geometry_coord.z, ent_inct.uv, ent_inct.wr);

        const auto bsdf_sample = ent_shd.bsdf->sample(ent_inct.wr, TransportMode::Radiance, sampler.sample3());
        if(!bsdf_sample.f)
            return pixel;

        coef *= bsdf_sample.f * std::abs(cos(ent_inct.geometry_coord.z, bsdf_sample.dir)) / bsdf_sample.pdf;
        r = Ray(ent_inct.eps_offset(bsdf_sample.dir), bsdf_sample.dir);
    }

    return pixel;
}

Pixel trace_ao(const AOParams &params, const Scene &scene, const Ray &ray, Sampler &sampler)
{
    EntityIntersection inct;
    if(!scene.closest_intersection(ray, &inct))
        return { params.background_color, {}, {}, 1 };

    Spectrum pixel_albedo = params.high_color;
    Vec3     pixel_normal = inct.geometry_coord.z;
    real     pixel_denoise = inct.entity->get_no_denoise_flag() ? real(0) : real(1);;

    const Vec3 start_pos = inct.eps_offset(inct.geometry_coord.z);

    real ao_factor = 0;
    for(int i = 0; i < params.ao_sample_count; ++i)
    {
        const Sample2 sam = sampler.sample2();
        const Vec3 local_dir = math::distribution::zweighted_on_hemisphere(sam.u, sam.v).first;
        const Vec3 global_dir = inct.geometry_coord.local_to_global(local_dir).normalize();

        const Vec3 end_pos = start_pos + global_dir * params.max_occlusion_distance;
        if(scene.visible(start_pos, end_pos))
            ao_factor += 1;
    }
    ao_factor /= params.ao_sample_count;

    return {
        lerp(params.low_color, params.high_color, math::saturate(ao_factor)),
        pixel_albedo, pixel_normal, pixel_denoise
    };
}

Pixel trace_albedo_ao(const AlbedoAOParams &params, const Scene &scene, const Ray &ray, Sampler &sampler, Arena &arena)
{
    Pixel pixel;

    EntityIntersection inct;
    if(!scene.closest_intersection(ray, &inct))
    {
        const EnvirLight *env = scene.envir_light();
        pixel.value = env ? env->radiance(ray.o, ray.d) : Spectrum();
        return pixel;
    }

    Spectrum le;
    if(const AreaLight *light = inct.entity->as_light())
        le = light->radiance(inct.pos, inct.geometry_coord.z, inct.uv, inct.wr);

    const ShadingPoint shd = inct.material->shade(inct, arena);
    const Spectrum albedo = shd.bsdf->albedo();

    pixel.normal = inct.geometry_coord.z;
    pixel.albedo = shd.bsdf->albedo();
    pixel.denoise = inct.entity->get_no_denoise_flag() ? real(0) : real(1);

    const Vec3 start_pos = inct.eps_offset(inct.geometry_coord.z);

    real ao_factor = 0;
    for(int i = 0; i < params.ao_sample_count; ++i)
    {
        const Sample2 sam = sampler.sample2();
        const Vec3 local_dir = math::distribution::zweighted_on_hemisphere(sam.u, sam.v).first;
        const Vec3 global_dir = inct.geometry_coord.local_to_global(local_dir).normalize();

        const Vec3 end_pos = start_pos + global_dir * params.max_occlusion_distance;
        if(scene.visible(start_pos, end_pos))
            ao_factor += 1;
    }
    ao_factor /= params.ao_sample_count;

    pixel.value = le + ao_factor * albedo;
    return pixel;
}

AGZ_TRACER_RENDER_END
