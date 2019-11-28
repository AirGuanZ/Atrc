
#include <agz/tracer/core/bsdf.h>
#include <agz/tracer/core/entity.h>
#include <agz/tracer/core/light.h>
#include <agz/tracer/core/medium.h>
#include <agz/tracer/core/scene.h>

#include "./mis_light_bsdf.h"

AGZ_TRACER_BEGIN

Spectrum mis_sample_area_light(
    const Scene &scene, const AreaLight *light, const EntityIntersection &inct, const ShadingPoint &shd, const Sample5 &sam)
{
    auto light_sample = light->sample(inct.pos, sam);
    if(!light_sample.radiance || !light_sample.pdf)
        return {};

    real shadow_ray_len = (light_sample.pos - inct.pos).length() - EPS;
    if(shadow_ray_len <= EPS)
        return {};
    Vec3 inct_to_light = (light_sample.pos - inct.pos).normalize();
    Ray shadow_ray(inct.pos, inct_to_light, EPS, shadow_ray_len);
    if(scene.has_intersection(shadow_ray))
        return {};

    auto bsdf_f = shd.bsdf->eval(inct_to_light, inct.wr, TM_Radiance);
    if(!bsdf_f)
        return {};

    Spectrum f = light_sample.radiance * bsdf_f * std::abs(cos(inct_to_light, inct.geometry_coord.z));

    if(light_sample.is_delta)
        return f / light_sample.pdf;
    real bsdf_pdf = shd.bsdf->pdf(inct_to_light, inct.wr, TM_Radiance);
    return f / (light_sample.pdf + bsdf_pdf);
}

Spectrum mis_sample_area_light(
    const Scene &scene, const AreaLight *light, const MediumScattering &scattering, const ShadingPoint &shd, const Sample5 &sam)
{
    auto light_sample = light->sample(scattering.pos, sam);
    if(!light_sample.radiance || !light_sample.pdf)
        return {};

    if(scene.visible(light_sample.pos, scattering.pos))
        return {};

    Vec3 inct_to_light = (light_sample.pos - scattering.pos).normalize();
    auto bsdf_f = shd.bsdf->eval(inct_to_light, scattering.wr, TM_Radiance);
    if(!bsdf_f)
        return {};
    Spectrum f = light_sample.radiance * bsdf_f;

    if(light_sample.is_delta)
        return f / light_sample.pdf;
    real bsdf_pdf = shd.bsdf->pdf(inct_to_light, scattering.wr, TM_Radiance);
    return f / (light_sample.pdf + bsdf_pdf);
}

Spectrum mis_sample_nonarea_light(
    const Scene &scene, const NonareaLight *light, const EntityIntersection &inct, const ShadingPoint &shd, const Sample5 &sam)
{
    LightSampleResult light_sample = light->sample(inct.pos, sam);
    if(!light_sample.radiance)
        return {};

    if(!scene.visible(inct.pos, light_sample.pos))
        return {};

    Vec3 ref_to_light = light_sample.ref_to_light();
    Spectrum bsdf_f = shd.bsdf->eval(ref_to_light, inct.wr, TM_Radiance);
    if(!bsdf_f)
        return {};

    Spectrum f = light_sample.radiance * bsdf_f * std::abs(cos(ref_to_light, inct.geometry_coord.z));

    if(light_sample.is_delta)
        return f / light_sample.pdf;

    real bsdf_pdf = shd.bsdf->pdf(ref_to_light, inct.wr, TM_Radiance);
    return f / (light_sample.pdf + bsdf_pdf);
}

Spectrum mis_sample_nonarea_light(
    const Scene &scene, const NonareaLight *light, const MediumScattering &scattering, const ShadingPoint &shd, const Sample5 &sam)
{
    LightSampleResult light_sample = light->sample(scattering.pos, sam);
    if(!light_sample.radiance)
        return {};

    if(!scene.visible(scattering.pos, light_sample.pos))
        return {};

    Vec3 ref_to_light = light_sample.ref_to_light();
    Spectrum bsdf_f = shd.bsdf->eval(ref_to_light, scattering.wr, TM_Radiance);
    if(!bsdf_f)
        return {};
    Spectrum f = light_sample.radiance * bsdf_f;

    if(light_sample.is_delta)
        return f / light_sample.pdf;

    real bsdf_pdf = shd.bsdf->pdf(ref_to_light, scattering.wr, TM_Radiance);
    return f / (light_sample.pdf + bsdf_pdf);
}

Spectrum mis_sample_light(
    const Scene &scene, const Light *lht, const EntityIntersection &inct, const ShadingPoint &shd, const Sample5 &sam)
{
    if(lht->is_area())
        return mis_sample_area_light(scene, lht->as_area(), inct, shd, sam);
    return mis_sample_nonarea_light(scene, lht->as_nonarea(), inct, shd, sam);
}

Spectrum mis_sample_light(
    const Scene &scene, const Light *lht, const MediumScattering &scattering, const ShadingPoint &shd, const Sample5 &sam)
{
    if(lht->is_area())
        return mis_sample_area_light(scene, lht->as_area(), scattering, shd, sam);
    return mis_sample_nonarea_light(scene, lht->as_nonarea(), scattering, shd, sam);
}

Spectrum mis_sample_bsdf(
    const Scene &scene, const EntityIntersection &inct, const ShadingPoint &shd, const Sample3 &sam,
    BSDFSampleResult &bsdf_sample, bool &has_ent_inct, EntityIntersection &ent_inct)
{
    has_ent_inct = false;

    bsdf_sample = shd.bsdf->sample(inct.wr, TM_Radiance, sam);
    if(!bsdf_sample.f)
        return {};
    bsdf_sample.dir = bsdf_sample.dir.normalize();

    Ray new_ray(inct.eps_offset(bsdf_sample.dir), bsdf_sample.dir);
    has_ent_inct = scene.closest_intersection(new_ray, &ent_inct);

    Spectrum nonarea_illum;
    const Medium *medium = inct.medium(bsdf_sample.dir);

    if(has_ent_inct)
    {
        real new_inct_dist = ent_inct.t;
        real new_inct_dist2 = new_inct_dist * new_inct_dist;

        for(auto light : scene.nonarea_lights())
        {
            Vec3 light_pnt;
            Spectrum light_radiance = light->radiance(new_ray.o, new_ray.d, &light_pnt);
            if(!light_radiance || (light_pnt - new_ray.o).length_square() >= new_inct_dist2)
                continue;

            Spectrum tr = medium->tr(new_ray.o, light_pnt);
            Spectrum f = tr * light_radiance * bsdf_sample.f * std::abs(dot(inct.geometry_coord.z, new_ray.d));

            if(bsdf_sample.is_delta)
                nonarea_illum += f / bsdf_sample.pdf;
            else
            {
                real light_pdf = light->pdf(new_ray.o, new_ray.d);
                nonarea_illum += f / (bsdf_sample.pdf + light_pdf);
            }
        }
    }
    else
    {
        for(auto light : scene.nonarea_lights())
        {
            Spectrum light_radiance = light->radiance(new_ray.o, new_ray.d);
            if(!light_radiance)
                continue;

            // 没有交点，故可以假设没有medium，也就不需要计算tr
            Spectrum f = light_radiance * bsdf_sample.f * std::abs(dot(inct.geometry_coord.z, new_ray.d));

            if(bsdf_sample.is_delta)
                nonarea_illum += f / bsdf_sample.pdf;
            else
            {
                real light_pdf = light->pdf(new_ray.o, new_ray.d);
                nonarea_illum += f / (bsdf_sample.pdf + light_pdf);
            }
        }
    }

    if(!has_ent_inct)
        return nonarea_illum;

    auto light = ent_inct.entity->as_light();
    if(!light)
        return nonarea_illum;

    Spectrum light_radiance = light->radiance(ent_inct, ent_inct.wr);
    if(!light_radiance)
        return nonarea_illum;

    Spectrum tr = medium->tr(new_ray.o, ent_inct.pos);
    Spectrum f = tr * light_radiance * bsdf_sample.f * std::abs(dot(inct.geometry_coord.z, new_ray.d));

    if(bsdf_sample.is_delta)
        return nonarea_illum + f / bsdf_sample.pdf;

    real light_pdf = light->pdf(new_ray.o, ent_inct);
    return nonarea_illum + f / (bsdf_sample.pdf + light_pdf);
}

Spectrum mis_sample_bsdf(
    const Scene &scene, const MediumScattering &scattering, const ShadingPoint &shd, const Sample3 &sam,
    BSDFSampleResult &bsdf_sample, bool &has_ent_inct, EntityIntersection &ent_inct)
{
    has_ent_inct = false;

    bsdf_sample = shd.bsdf->sample(scattering.wr, TM_Radiance, sam);
    if(!bsdf_sample.f)
        return {};
    bsdf_sample.dir = bsdf_sample.dir.normalize();

    Ray new_ray(scattering.pos, bsdf_sample.dir);
    has_ent_inct = scene.closest_intersection(new_ray, &ent_inct);

    Spectrum nonarea_illum;
    const Medium *medium = scattering.medium;

    if(has_ent_inct)
    {
        real new_inct_dist = ent_inct.t;
        real new_inct_dist2 = new_inct_dist * new_inct_dist;

        for(auto light : scene.nonarea_lights())
        {
            Vec3 light_pnt;
            Spectrum light_f = light->radiance(new_ray.o, new_ray.d, &light_pnt);
            if(!light_f || (light_pnt - new_ray.o).length() >= new_inct_dist2)
                continue;

            Spectrum tr = medium->tr(new_ray.o, light_pnt);
            Spectrum f = tr * light_f * bsdf_sample.f;

            if(bsdf_sample.is_delta)
                nonarea_illum += f / bsdf_sample.pdf;
            else
            {
                real light_pdf = light->pdf(new_ray.o, new_ray.d);
                nonarea_illum += f / (bsdf_sample.pdf + light_pdf);
            }
        }
    }
    else
    {
        for(auto light : scene.nonarea_lights())
        {
            Spectrum light_f = light->radiance(new_ray.o, new_ray.d);
            if(!light_f)
                continue;

            // 没有交点，故可以假设没有medium，也就不需要计算tr
            Spectrum f = light_f * bsdf_sample.f;

            if(bsdf_sample.is_delta)
                nonarea_illum += f / bsdf_sample.pdf;
            else
            {
                real light_pdf = light->pdf(new_ray.o, new_ray.d);
                nonarea_illum += f / (bsdf_sample.pdf + light_pdf);
            }
        }
    }

    if(!has_ent_inct)
        return nonarea_illum;

    auto light = ent_inct.entity->as_light();
    if(!light)
        return nonarea_illum;

    Spectrum light_f = light->radiance(ent_inct, ent_inct.wr);
    if(!light_f)
        return nonarea_illum;

    Spectrum tr = medium->tr(new_ray.o, ent_inct.pos);
    Spectrum f = tr * light_f * bsdf_sample.f;

    if(bsdf_sample.is_delta)
        return nonarea_illum + f / bsdf_sample.pdf;

    real light_pdf = light->pdf(new_ray.o, ent_inct);
    return nonarea_illum + f / (bsdf_sample.pdf + light_pdf);
}

Spectrum mis_sample_bsdf(const Scene &scene, const EntityIntersection &inct, const ShadingPoint &shd, const Sample3 &sam)
{
    BSDFSampleResult bsdf_sample;
    bool has_ent_inct;
    EntityIntersection ent_inct;
    return mis_sample_bsdf(scene, inct, shd, sam, bsdf_sample, has_ent_inct, ent_inct);
}

Spectrum mis_sample_bsdf(const Scene &scene, const MediumScattering &scattering, const ShadingPoint &shd, const Sample3 &sam)
{
    BSDFSampleResult bsdf_sample;
    bool has_ent_inct;
    EntityIntersection ent_inct;
    return mis_sample_bsdf(scene, scattering, shd, sam, bsdf_sample, has_ent_inct, ent_inct);
}

AGZ_TRACER_END
