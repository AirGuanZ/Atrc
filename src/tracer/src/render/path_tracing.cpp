#include <agz/tracer/core/bsdf.h>
#include <agz/tracer/core/bssrdf.h>
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

Pixel trace_std(
    const TraceParams &params, const Scene &scene, const Ray &ray,
    Sampler &sampler, Arena &arena)
{
    FSpectrum coef(1);
    Ray r = ray;

    Pixel pixel;

    int scattering_count = 0;

    for(int depth = 1, s_depth = 1; depth <= params.max_depth; ++depth)
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
            const auto medium_sample = medium->sample_scattering(
                r.o, ent_inct.pos, sampler, arena);

            // tr is accounted here
            coef *= medium_sample.throughput;

            // process medium scattering

            if(medium_sample.is_scattering_happened())
            {
                ++scattering_count;

                const auto &scattering_point = medium_sample.scattering_point;
                const auto phase_function = medium_sample.phase_function;

                // compute direct illumination

                FSpectrum direct_illum;
                for(int i = 0; i < params.direct_illum_sample_count; ++i)
                {
                    for(auto light : scene.lights())
                    {
                        direct_illum += coef * mis_sample_light(
                            scene, light, scattering_point, phase_function, sampler);
                    }
                    direct_illum += coef * mis_sample_bsdf(
                        scene, scattering_point, phase_function, sampler);
                }

                pixel.value += direct_illum / real(params.direct_illum_sample_count);

                // sample phase function

                const auto bsdf_sample = phase_function->sample_all(
                    scattering_point.wr, TransMode::Radiance, sampler.sample3());
                if(!bsdf_sample.f || bsdf_sample.pdf < EPS())
                    return pixel;

                r = Ray(scattering_point.pos, bsdf_sample.dir.normalize());
                coef *= bsdf_sample.f / bsdf_sample.pdf;
                continue;
            }
        }
        else
        {
            // continus scattering count is too large
            // only account absorbtion here
            const FSpectrum ab = medium->ab(r.o, ent_inct.pos, sampler);
            coef *= ab;
        }

        scattering_count = 0;

        // process surface scattering

        if(depth == 1)
        {
            if(auto light = ent_inct.entity->as_light())
            {
                pixel.value += coef * light->radiance(
                    ent_inct.pos, ent_inct.geometry_coord.z, ent_inct.uv, ent_inct.wr);
            }
        }

        // direct illumination

        FSpectrum direct_illum;
        for(int i = 0; i < params.direct_illum_sample_count; ++i)
        {
            for(auto light : scene.lights())
            {
                direct_illum += coef * mis_sample_light(
                    scene, light, ent_inct, ent_shd, sampler);
            }
            direct_illum += coef * mis_sample_bsdf(
                scene, ent_inct, ent_shd, sampler);
        }

        pixel.value += real(1) / params.direct_illum_sample_count * direct_illum;

        // sample bsdf

        auto bsdf_sample = ent_shd.bsdf->sample_all(
            ent_inct.wr, TransMode::Radiance, sampler.sample3());
        if(!bsdf_sample.f || bsdf_sample.pdf < EPS())
            return pixel;

        bool is_new_sample_delta = bsdf_sample.is_delta;
        AGZ_SCOPE_EXIT{
            if(is_new_sample_delta && depth >= 2 && s_depth <= params.specular_depth)
            {
                --depth;
                ++s_depth;
            }
        };

        const real abscos = std::abs(cos(
            ent_inct.geometry_coord.z, bsdf_sample.dir));
        coef *= bsdf_sample.f * abscos / bsdf_sample.pdf;

        r = Ray(ent_inct.eps_offset(bsdf_sample.dir),
                bsdf_sample.dir.normalize());

        // bssrdf

        if(!ent_shd.bssrdf)
            continue;

        const bool pos_in = ent_inct.geometry_coord.in_positive_z_hemisphere(
            bsdf_sample.dir);
        const bool pos_out = ent_inct.geometry_coord.in_positive_z_hemisphere(
            ent_inct.wr);

        if(!pos_in && pos_out)
        {
            const auto bssrdf_sample = ent_shd.bssrdf->sample_pi(
                sampler.sample3(), arena);
            if(!bssrdf_sample.coef)
                return pixel;

            coef *= bssrdf_sample.coef / bssrdf_sample.pdf;

            auto &new_inct = bssrdf_sample.inct;
            auto new_shd = new_inct.material->shade(new_inct, arena);

            FSpectrum new_direct_illum;
            for(int i = 0; i < params.direct_illum_sample_count; ++i)
            {
                for(auto light : scene.lights())
                {
                    new_direct_illum += coef * mis_sample_light(
                        scene, light, new_inct, new_shd, sampler);
                }
                new_direct_illum += coef * mis_sample_bsdf(
                    scene, new_inct, new_shd, sampler);
            }

            pixel.value += real(1) / params.direct_illum_sample_count
                         * new_direct_illum;

            const auto new_bsdf_sample = new_shd.bsdf->sample_all(
                new_inct.wr, TransMode::Radiance, sampler.sample3());
            if(!new_bsdf_sample.f)
                return pixel;

            const real new_abscos = std::abs(cos(
                new_inct.geometry_coord.z, new_bsdf_sample.dir));
            coef *= new_bsdf_sample.f * new_abscos / new_bsdf_sample.pdf;

            r = Ray(new_inct.eps_offset(new_bsdf_sample.dir),
                    new_bsdf_sample.dir.normalize());

            is_new_sample_delta = new_bsdf_sample.is_delta;
        }
    }

    return pixel;
}

Pixel trace_nomis(
    const TraceParams &params, const Scene &scene, const Ray &ray,
    Sampler &sampler, Arena &arena)
{
    FSpectrum coef(1);
    Ray r = ray;

    Pixel pixel;

    int scattering_count = 0;

    for(int depth = 1, s_depth = 1; depth <= params.max_depth; ++depth)
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
            const auto medium_sample = medium->sample_scattering(
                r.o, ent_inct.pos, sampler, arena);
            coef *= medium_sample.throughput;

            // process medium scattering

            if(medium_sample.is_scattering_happened())
            {
                ++scattering_count;

                const auto &scattering_point = medium_sample.scattering_point;
                const auto phase_function = medium_sample.phase_function;

                const auto phase_sample = phase_function->sample_all(
                    ent_inct.wr, TransMode::Radiance, sampler.sample3());
                if(!phase_sample.f)
                    return pixel;

                coef *= phase_sample.f / phase_sample.pdf;
                r = Ray(scattering_point.pos, phase_sample.dir);

                continue;
            }
        }
        else
        {
            // continus scattering count is too large
            // only account absorbtion here
            const FSpectrum ab = medium->ab(r.o, ent_inct.pos, sampler);
            coef *= ab;
        }

        // surface intersection is sampled. clear continus scattering_count

        scattering_count = 0;

        // process surface scattering

        if(auto light = ent_inct.entity->as_light())
        {
            pixel.value += coef * light->radiance(
                ent_inct.pos, ent_inct.geometry_coord.z,
                ent_inct.uv, ent_inct.wr);
        }

        const auto bsdf_sample = ent_shd.bsdf->sample_all(
            ent_inct.wr, TransMode::Radiance, sampler.sample3());
        if(!bsdf_sample.f)
            return pixel;

        const real abscos = std::abs(cos(
            ent_inct.geometry_coord.z, bsdf_sample.dir));
        coef *= bsdf_sample.f * abscos / bsdf_sample.pdf;
        r = Ray(ent_inct.eps_offset(bsdf_sample.dir), bsdf_sample.dir);

        bool is_new_sample_specular = bsdf_sample.is_delta;
        AGZ_SCOPE_EXIT{
            if(is_new_sample_specular && depth >= 2 &&
               s_depth <= params.specular_depth)
            {
                --depth;
                ++s_depth;
            }
        };

        // bssrdf

        if(!ent_shd.bssrdf)
            continue;

        const bool pos_in = ent_inct.geometry_coord.in_positive_z_hemisphere(
            bsdf_sample.dir);
        const bool pos_out = ent_inct.geometry_coord.in_positive_z_hemisphere(
            ent_inct.wr);

        if(!pos_in && pos_out)
        {
            const auto bssrdf_sample = ent_shd.bssrdf->sample_pi(
                sampler.sample3(), arena);
            if(!bssrdf_sample.coef)
                return pixel;

            coef *= bssrdf_sample.coef / bssrdf_sample.pdf;

            auto &new_inct = bssrdf_sample.inct;
            auto new_shd = new_inct.material->shade(new_inct, arena);

            const auto new_bsdf_sample = new_shd.bsdf->sample_all(
                new_inct.wr, TransMode::Radiance, sampler.sample3());
            if(!new_bsdf_sample.f)
                return pixel;

            const real new_abscos = std::abs(cos(
                new_inct.geometry_coord.z, new_bsdf_sample.dir));
            coef *= new_bsdf_sample.f * new_abscos / new_bsdf_sample.pdf;

            r = Ray(new_inct.eps_offset(new_bsdf_sample.dir),
                new_bsdf_sample.dir.normalize());

            is_new_sample_specular = new_bsdf_sample.is_delta;
        }
    }

    return pixel;
}

Pixel trace_ao(
    const AOParams &params, const Scene &scene, const Ray &ray, Sampler &sampler)
{
    EntityIntersection inct;
    if(!scene.closest_intersection(ray, &inct))
        return { { {}, {}, 1 }, params.background_color };

    FSpectrum pixel_albedo = params.high_color;
    FVec3    pixel_normal  = inct.geometry_coord.z;
    real     pixel_denoise = inct.entity->get_no_denoise_flag() ? real(0) : real(1);;

    const FVec3 start_pos = inct.eps_offset(inct.geometry_coord.z);

    real ao_factor = 0;
    for(int i = 0; i < params.ao_sample_count; ++i)
    {
        const Sample2 sam = sampler.sample2();
        const FVec3 local_dir = math::distribution
                                    ::zweighted_on_hemisphere(sam.u, sam.v).first;
        const FVec3 global_dir = inct.geometry_coord.local_to_global(local_dir)
                                                   .normalize();

        const FVec3 end_pos = start_pos + global_dir * params.max_occlusion_distance;
        if(scene.visible(start_pos, end_pos))
            ao_factor += 1;
    }
    ao_factor /= params.ao_sample_count;

    return {
        { pixel_albedo, pixel_normal, pixel_denoise },
        lerp(params.low_color, params.high_color, math::saturate(ao_factor))
    };
}

Pixel trace_albedo_ao(
    const AlbedoAOParams &params, const Scene &scene, const Ray &ray,
    Sampler &sampler, Arena &arena)
{
    Pixel pixel;

    EntityIntersection inct;
    if(!scene.closest_intersection(ray, &inct))
    {
        const EnvirLight *env = scene.envir_light();
        pixel.value = env ? env->radiance(ray.o, ray.d) : FSpectrum();
        return pixel;
    }

    FSpectrum le;
    if(const AreaLight *light = inct.entity->as_light())
        le = light->radiance(inct.pos, inct.geometry_coord.z, inct.uv, inct.wr);

    const ShadingPoint shd = inct.material->shade(inct, arena);
    const FSpectrum albedo = shd.bsdf->albedo();

    pixel.normal = inct.geometry_coord.z;
    pixel.albedo = shd.bsdf->albedo();
    pixel.denoise = inct.entity->get_no_denoise_flag() ? real(0) : real(1);

    const FVec3 start_pos = inct.eps_offset(inct.geometry_coord.z);

    real ao_factor = 0;
    for(int i = 0; i < params.ao_sample_count; ++i)
    {
        const Sample2 sam = sampler.sample2();
        const FVec3 local_dir = math::distribution
                                    ::zweighted_on_hemisphere(sam.u, sam.v).first;
        const FVec3 global_dir = inct.geometry_coord.local_to_global(local_dir)
                                                   .normalize();

        const FVec3 end_pos = start_pos + global_dir * params.max_occlusion_distance;
        if(scene.visible(start_pos, end_pos))
            ao_factor += 1;
    }
    ao_factor /= params.ao_sample_count;

    pixel.value = le + ao_factor * albedo;
    return pixel;
}

AGZ_TRACER_RENDER_END
