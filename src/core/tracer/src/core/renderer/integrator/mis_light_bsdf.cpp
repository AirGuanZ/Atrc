
#include <agz/tracer/core/bsdf.h>
#include <agz/tracer/core/entity.h>
#include <agz/tracer/core/light.h>
#include <agz/tracer/core/scene.h>

#include "./mis_light_bsdf.h"

AGZ_TRACER_BEGIN

Spectrum mis_sample_area_light(const Scene &scene, const AreaLight *light, const EntityIntersection &inct, const ShadingPoint &shd, const Sample5 &sam)
{
    auto light_sample = light->sample(inct.pos, sam);
    if(!light_sample.radiance || !light_sample.pdf)
        return Spectrum();

    real shadow_ray_len = (light_sample.pos - inct.pos).length() - EPS;
    if(shadow_ray_len <= EPS)
        return Spectrum();
    Vec3 inct_to_light = (light_sample.pos - inct.pos).normalize();
    Ray shadow_ray(inct.pos, inct_to_light, EPS, shadow_ray_len);
    if(scene.has_intersection(shadow_ray))
        return Spectrum();

    auto bsdf_f = shd.bsdf->eval(inct_to_light, inct.wr, TM_Radiance);
    if(!bsdf_f)
        return Spectrum();

    Spectrum f = light_sample.radiance * bsdf_f * std::abs(cos(inct_to_light, inct.geometry_coord.z));

    if(light_sample.is_delta)
        return f / light_sample.pdf;
    real bsdf_pdf = shd.bsdf->pdf(inct_to_light, inct.wr, TM_Radiance);
    return f / (light_sample.pdf + bsdf_pdf);
}

Spectrum mis_sample_area_light(const Scene &scene, const AreaLight *light, const ScatteringPoint &pnt, const Sample5 sam)
{
    Vec3 pos = pnt.pos();

    auto light_sample = light->sample(pos, sam);
    if(!light_sample.radiance || light_sample.pdf < EPS)
        return Spectrum();

    if(!scene.visible(light_sample.pos, pos))
        return Spectrum();

    Vec3 pnt_to_light = (light_sample.pos - pos).normalize();
    auto bsdf_f = pnt.eval(pnt_to_light, pnt.wr(), TM_Radiance);
    if(!bsdf_f)
        return Spectrum();

    auto medium = pnt.medium(pnt_to_light);
    Spectrum tr = medium->tr(light_sample.pos, pos);

    Spectrum f = tr * light_sample.radiance * bsdf_f * pnt.proj_wi_factor(pnt_to_light);
    if(light_sample.is_delta)
        return f / light_sample.pdf;

    real pnt_pdf = pnt.pdf(pnt_to_light, pnt.wr(), TM_Radiance);
    return f / (light_sample.pdf + pnt_pdf);
}

Spectrum mis_sample_nonarea_light(const Scene &scene, const NonareaLight *light, const EntityIntersection &inct, const ShadingPoint &shd, const Sample5 &sam)
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

Spectrum mis_sample_nonarea_light(const Scene &scene, const NonareaLight *light, const ScatteringPoint &pnt, const Sample5 &sam)
{
    Vec3 pos = pnt.pos();
    LightSampleResult light_sample = light->sample(pos, sam);
    if(!light_sample.radiance)
        return {};

    if(!scene.visible(pos, light_sample.pos))
        return {};

    Vec3 ref_to_light = light_sample.ref_to_light();
    Spectrum bsdf_f = pnt.eval(ref_to_light, pnt.wr(), TM_Radiance);
    if(!bsdf_f)
        return {};

    Spectrum f = light_sample.radiance * bsdf_f * pnt.proj_wi_factor(ref_to_light);
    if(light_sample.is_delta)
        return f / light_sample.pdf;

    real pnt_pdf = pnt.pdf(ref_to_light, pnt.wr(), TM_Radiance);
    return f / (light_sample.pdf + pnt_pdf);
}

Spectrum mis_sample_light(const Scene &scene, const Light *lht, const EntityIntersection &inct, const ShadingPoint &shd, const Sample5 &sam)
{
    if(lht->is_area())
        return mis_sample_area_light(scene, lht->as_area(), inct, shd, sam);
    return mis_sample_nonarea_light(scene, lht->as_nonarea(), inct, shd, sam);
}

Spectrum mis_sample_light(const Scene &scene, const Light *lht, const ScatteringPoint &pnt, const Sample5 sam)
{
    if(lht->is_area())
        return mis_sample_area_light(scene, lht->as_area(), pnt, sam);
    return mis_sample_nonarea_light(scene, lht->as_nonarea(), pnt, sam);
}

Spectrum mis_sample_scattering(const Scene &scene, const ScatteringPoint &pnt, const Sample3 &sam)
{
    auto pnt_sample = pnt.sample(pnt.wr(), sam, TM_Radiance);
    if(!pnt_sample.f || pnt_sample.pdf < EPS)
        return Spectrum();

    Ray new_ray(pnt.pos(), pnt_sample.dir.normalize(), EPS);
    EntityIntersection new_inct;
    bool has_new_inct = scene.closest_intersection(new_ray, &new_inct);

    Spectrum nonarea_illum;
    auto medium = pnt.medium(pnt_sample.dir);
    
    if(has_new_inct)
    {
        real new_inct_dist = new_inct.t;

        for(auto light : scene.nonarea_lights())
        {
            Vec3 light_pnt;
            Spectrum light_f = light->radiance(new_ray.o, new_ray.d, &light_pnt);
            if(!light_f || (light_pnt - new_ray.o).length() >= new_inct_dist)
                continue;

            Spectrum tr = medium->tr(new_ray.o, light_pnt);
            Spectrum f = tr * light_f * pnt_sample.f * pnt.proj_wi_factor(new_ray.d);

            if(pnt_sample.is_delta)
                nonarea_illum += f / pnt_sample.pdf;
            else
            {
                real light_pdf = light->pdf(new_ray.o, new_ray.d);
                nonarea_illum += f / (pnt_sample.pdf + light_pdf);
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

            Spectrum f = light_f * pnt_sample.f * pnt.proj_wi_factor(new_ray.d);

            if(pnt_sample.is_delta)
                nonarea_illum += f / pnt_sample.pdf;
            else
            {
                real light_pdf = light->pdf(new_ray.o, new_ray.d);
                nonarea_illum += f / (pnt_sample.pdf + light_pdf);
            }
        }
    }

    if(!has_new_inct)
        return nonarea_illum;

    auto light = new_inct.entity->as_light();
    if(!light)
        return nonarea_illum;

    Spectrum light_f = light->radiance(new_inct, new_inct.wr);
    if(!light_f)
        return nonarea_illum;

    Spectrum tr = medium->tr(pnt.pos(), new_inct.pos);

    Spectrum f = tr * light_f * pnt_sample.f * pnt.proj_wi_factor(pnt_sample.dir);
    if(pnt_sample.is_delta)
        return nonarea_illum + f / pnt_sample.pdf;

    real light_pdf = light->pdf(pnt.pos(), new_inct);
    return nonarea_illum + f / (pnt_sample.pdf + light_pdf);
}

Spectrum mis_sample_bsdf(
    const Scene &scene, const EntityIntersection &inct, const ShadingPoint &shd, const Sample3 &sam,
    BSDFSampleResult &bsdf_sample, bool &has_new_inct, EntityIntersection &new_inct)
{
    has_new_inct = false;

    bsdf_sample = shd.bsdf->sample(inct.wr, TM_Radiance, sam);
    if(!bsdf_sample.f)
        return {};
    bsdf_sample.dir = bsdf_sample.dir.normalize();

    Ray new_ray(inct.eps_offset(bsdf_sample.dir), bsdf_sample.dir);
    has_new_inct = scene.closest_intersection(new_ray, &new_inct);

    Spectrum nonarea_illum;
    const Medium *medium = inct.medium(bsdf_sample.dir);

    if(has_new_inct)
    {
        real new_inct_dist = new_inct.t;
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

    if(!has_new_inct)
        return nonarea_illum;

    auto light = new_inct.entity->as_light();
    if(!light)
        return nonarea_illum;

    Spectrum light_radiance = light->radiance(new_inct, new_inct.wr);
    if(!light_radiance)
        return nonarea_illum;

    Spectrum tr = medium->tr(new_ray.o, new_inct.pos);
    Spectrum f = tr * light_radiance * bsdf_sample.f * std::abs(dot(inct.geometry_coord.z, new_ray.d));

    if(bsdf_sample.is_delta)
        return nonarea_illum + f / bsdf_sample.pdf;

    real light_pdf = light->pdf(new_ray.o, new_inct);
    return nonarea_illum + f / (bsdf_sample.pdf + light_pdf);
}

AGZ_TRACER_END
