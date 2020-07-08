
#include <agz/tracer/core/bsdf.h>
#include <agz/tracer/core/entity.h>
#include <agz/tracer/core/light.h>
#include <agz/tracer/core/medium.h>
#include <agz/tracer/core/scene.h>
#include <agz/tracer/render/direct_illum.h>

AGZ_TRACER_BEGIN

Spectrum mis_sample_area_light(
    const Scene &scene, const AreaLight *light,
    const EntityIntersection &inct, const ShadingPoint &shd,
    Sampler &sampler)
{
    const Sample5 sam = sampler.sample5();

    const auto light_sample = light->sample(inct.pos, sam);
    if(!light_sample.radiance || !light_sample.pdf)
        return {};

    const real shadow_ray_len = (light_sample.pos - inct.pos).length() - EPS();
    if(shadow_ray_len <= EPS())
        return {};
    const FVec3 inct_to_light = (light_sample.pos - inct.pos).normalize();
    const Ray shadow_ray(inct.pos, inct_to_light, EPS(), shadow_ray_len);
    if(scene.has_intersection(shadow_ray))
        return {};

    const auto med = inct.medium(inct_to_light);

    const auto bsdf_f = shd.bsdf->eval_all(inct_to_light, inct.wr, TransMode::Radiance);
    if(!bsdf_f)
        return {};

    const Spectrum f = med->tr(light_sample.pos, inct.pos, sampler)
                     * light_sample.radiance * bsdf_f
                     * std::abs(cos(inct_to_light, inct.geometry_coord.z));
    const real bsdf_pdf = shd.bsdf->pdf_all(inct_to_light, inct.wr);

    return f / (light_sample.pdf + bsdf_pdf);
}

Spectrum mis_sample_area_light(
    const Scene &scene, const AreaLight *light,
    const MediumScattering &scattering, const BSDF *phase_function,
    Sampler &sampler)
{
    const Sample5 sam = sampler.sample5();

    const auto light_sample = light->sample(scattering.pos, sam);
    if(!light_sample.radiance || !light_sample.pdf)
        return {};

    if(!scene.visible(light_sample.pos, scattering.pos))
        return {};

    const FVec3 inct_to_light = (light_sample.pos - scattering.pos).normalize();
    auto bsdf_f = phase_function->eval_all(
        inct_to_light, scattering.wr, TransMode::Radiance);
    if(!bsdf_f)
        return {};

    const auto med = scattering.medium;
    const Spectrum f = med->tr(scattering.pos, light_sample.pos, sampler)
                     * light_sample.radiance * bsdf_f;
    const real bsdf_pdf = phase_function->pdf_all(inct_to_light, scattering.wr);

    return f / (light_sample.pdf + bsdf_pdf);
}

Spectrum mis_sample_envir_light(
    const Scene &scene, const EnvirLight *light,
    const EntityIntersection &inct, const ShadingPoint &shd,
    Sampler &sampler)
{
    const Sample5 sam = sampler.sample5();

    const auto light_sample = light->sample(inct.pos, sam);
    if(!light_sample.radiance)
        return {};

    if(!scene.visible(inct.pos, light_sample.pos))
        return {};

    const FVec3 ref_to_light = light_sample.ref_to_light();
    const Spectrum bsdf_f = shd.bsdf->eval_all(
        ref_to_light, inct.wr, TransMode::Radiance);
    if(!bsdf_f)
        return {};

    // no medium when envir light is visible

    const Spectrum f = light_sample.radiance
                     * bsdf_f * std::abs(cos(ref_to_light, inct.geometry_coord.z));
    const real bsdf_pdf = shd.bsdf->pdf_all(ref_to_light, inct.wr);

    return f / (light_sample.pdf + bsdf_pdf);
}

Spectrum mis_sample_envir_light(
    const Scene &scene, const EnvirLight *light,
    const MediumScattering &scattering, const BSDF *phase_function,
    Sampler &sampler)
{
    // there is no medium when envir light is visible
    return {};
}

Spectrum mis_sample_light(
    const Scene &scene, const Light *lht,
    const EntityIntersection &inct, const ShadingPoint &shd,
    Sampler &sampler)
{
    if(lht->is_area())
        return mis_sample_area_light(scene, lht->as_area(), inct, shd, sampler);
    return mis_sample_envir_light(scene, lht->as_envir(), inct, shd, sampler);
}

Spectrum mis_sample_light(
    const Scene &scene, const Light *lht,
    const MediumScattering &scattering, const BSDF *phase_function,
    Sampler &sampler)
{
    if(lht->is_area())
        return mis_sample_area_light(scene, lht->as_area(), scattering, phase_function, sampler);
    return mis_sample_envir_light(scene, lht->as_envir(), scattering, phase_function, sampler);
}

Spectrum mis_sample_bsdf(
    const Scene &scene, const EntityIntersection &inct, const ShadingPoint &shd, Sampler &sampler,
    BSDFSampleResult &bsdf_sample, bool &has_ent_inct, EntityIntersection &ent_inct)
{
    const Sample3 sam = sampler.sample3();
    has_ent_inct = false;

    bsdf_sample = shd.bsdf->sample_all(inct.wr, TransMode::Radiance, sam);
    if(!bsdf_sample.f)
        return {};
    bsdf_sample.dir = bsdf_sample.dir.normalize();

    const Ray new_ray(inct.eps_offset(bsdf_sample.dir), bsdf_sample.dir);
    has_ent_inct = scene.closest_intersection(new_ray, &ent_inct);

    const Medium *medium = inct.medium(bsdf_sample.dir);

    if(!has_ent_inct)
    {
        Spectrum envir_illum;

        if(auto light = scene.envir_light())
        {
            const Spectrum light_radiance = light->radiance(new_ray.o, new_ray.d);
            if(!light_radiance)
                return {};

            // no medium when there is no inct
            const Spectrum f = light_radiance
                             * bsdf_sample.f * std::abs(dot(inct.geometry_coord.z, new_ray.d));

            if(bsdf_sample.is_delta)
                envir_illum += f / bsdf_sample.pdf;
            else
            {
                real light_pdf = light->pdf(new_ray.o, new_ray.d);
                envir_illum += f / (bsdf_sample.pdf + light_pdf);
            }
        }

        return envir_illum;
    }

    auto light = ent_inct.entity->as_light();
    if(!light)
        return {};

    const Spectrum light_radiance = light->radiance(
        ent_inct.pos, ent_inct.geometry_coord.z, ent_inct.uv, ent_inct.wr);
    if(!light_radiance)
        return {};

    const Spectrum tr = medium->tr(new_ray.o, ent_inct.pos, sampler);
    const Spectrum f = tr * light_radiance * bsdf_sample.f
                     * std::abs(dot(inct.geometry_coord.z, new_ray.d));

    if(bsdf_sample.is_delta)
        return f / bsdf_sample.pdf;

    const real light_pdf = light->pdf(
        new_ray.o, ent_inct.pos, ent_inct.geometry_coord.z);
    return f / (bsdf_sample.pdf + light_pdf);
}

Spectrum mis_sample_bsdf(
    const Scene &scene, const MediumScattering &scattering, const BSDF *phase_function, Sampler &sampler,
    BSDFSampleResult &bsdf_sample, bool &has_ent_inct, EntityIntersection &ent_inct)
{
    const Sample3 sam = sampler.sample3();
    has_ent_inct = false;

    bsdf_sample = phase_function->sample_all(scattering.wr, TransMode::Radiance, sam);
    if(!bsdf_sample.f)
        return {};
    bsdf_sample.dir = bsdf_sample.dir.normalize();

    const Ray new_ray(scattering.pos, bsdf_sample.dir);
    has_ent_inct = scene.closest_intersection(new_ray, &ent_inct);

    const Medium *medium = scattering.medium;

    if(!has_ent_inct)
    {
        Spectrum envir_illum;

        if(auto light = scene.envir_light())
        {
            const Spectrum light_f = light->radiance(new_ray.o, new_ray.d);
            if(!light_f)
                return {};

            const Spectrum f = light_f * bsdf_sample.f;

            if(bsdf_sample.is_delta)
                envir_illum += f / bsdf_sample.pdf;
            else
            {
                const real light_pdf = light->pdf(new_ray.o, new_ray.d);
                envir_illum += f / (bsdf_sample.pdf + light_pdf);
            }
        }

        return envir_illum;
    }

    const auto light = ent_inct.entity->as_light();
    if(!light)
        return {};

    const Spectrum light_f = light->radiance(
        ent_inct.pos, ent_inct.geometry_coord.z, ent_inct.uv, ent_inct.wr);
    if(!light_f)
        return {};

    const Spectrum tr = medium->tr(new_ray.o, ent_inct.pos, sampler);
    const Spectrum f = tr * light_f * bsdf_sample.f;

    if(bsdf_sample.is_delta)
        return f / bsdf_sample.pdf;

    const real light_pdf = light->pdf(
        new_ray.o, ent_inct.pos, ent_inct.geometry_coord.z);
    return f / (bsdf_sample.pdf + light_pdf);
}

Spectrum mis_sample_bsdf(
    const Scene &scene,
    const EntityIntersection &inct, const ShadingPoint &shd,
    Sampler &sampler)
{
    BSDFSampleResult bsdf_sample(UNINIT);
    bool has_ent_inct;
    EntityIntersection ent_inct;
    return mis_sample_bsdf(
        scene, inct, shd, sampler,
        bsdf_sample, has_ent_inct, ent_inct);
}

Spectrum mis_sample_bsdf(
    const Scene &scene,
    const MediumScattering &scattering, const BSDF *phase_function,
    Sampler &sampler)
{
    BSDFSampleResult bsdf_sample(UNINIT);
    bool has_ent_inct;
    EntityIntersection ent_inct;
    return mis_sample_bsdf(
        scene, scattering, phase_function, sampler,
        bsdf_sample, has_ent_inct, ent_inct);
}

AGZ_TRACER_END
