#include <agz/tracer/core/bsdf.h>
#include <agz/tracer/core/entity.h>
#include <agz/tracer/core/light.h>
#include <agz/tracer/core/intersection.h>
#include <agz/tracer/core/material.h>
#include <agz/tracer/core/medium.h>
#include <agz/tracer/core/path_tracing_integrator.h>
#include <agz/tracer/core/sampler.h>
#include <agz/tracer/core/scene.h>
#include <agz/tracer/utility/direct_illum.h>
#include <agz/utility/misc.h>

AGZ_TRACER_BEGIN

class VolumetricPathTracingIntegrator : public PathTracingIntegrator
{
    int min_depth_ = 5;
    int max_depth_ = 10;
    real cont_prob_ = real(0.9);

public:

    void initialize(int min_depth, int max_depth, real cont_prob)
    {
        AGZ_HIERARCHY_TRY

        min_depth_ = min_depth;
        if(min_depth_ < 1)
            throw ObjectConstructionException("invalid min depth value: " + std::to_string(min_depth_));

        max_depth_ = max_depth;
        if(max_depth_ < min_depth_)
            throw ObjectConstructionException("invalid max depth value: " + std::to_string(max_depth_));

        cont_prob_ = cont_prob;
        if(cont_prob_ < 0 || cont_prob_ > 1)
            throw ObjectConstructionException("invalid continue prob value: " + std::to_string(cont_prob_));

        AGZ_HIERARCHY_WRAP("in initializing volumetric mis path tracing integrator")
    }

    Spectrum eval(GBufferPixel *gpixel, const Scene &scene, const Ray &ray, Sampler &sampler, Arena &arena) const override
    {
        Spectrum ret, coef(1);
        Ray r = ray;

        for(int depth = 1; depth <= max_depth_; ++depth)
        {
            // apply RR strategy

            if(depth > min_depth_)
            {
                if(sampler.sample1().u > cont_prob_)
                    return ret;
                coef /= cont_prob_;
            }

            // find closest entity intersection

            EntityIntersection ent_inct;
            bool has_ent_inct = scene.closest_intersection(r, &ent_inct);
            if(!has_ent_inct)
            {
                if(depth == 1)
                {
                    for(auto light : scene.nonarea_lights())
                        ret += coef * light->radiance(r.o, r.d);
                }
                return ret;
            }

            // fill gbuffer

            auto ent_shd = ent_inct.material->shade(ent_inct, arena);
            if(depth == 1 && gpixel)
            {
                gpixel->position = ent_inct.pos;
                gpixel->normal   = ent_shd.shading_normal;
                gpixel->albedo   = ent_shd.bsdf->albedo();
                gpixel->binary   = 1;
                gpixel->depth    = (r.o - ent_inct.pos).length();
                if(ent_inct.entity->get_no_denoise_flag())
                    gpixel->denoise = 0;
            }

            // sample medium scattering

            auto medium = ent_inct.wr_medium();
            auto medium_sample = medium->sample_scattering(r.o, ent_inct.pos, sampler, arena);

            coef *= medium_sample.throughput;

            // process medium scattering

            if(medium_sample.is_scattering_happened())
            {
                auto &scattering_point = medium_sample.scattering_point;
                auto phase_function = medium_sample.phase_function;

                // compute direct illumination

                Spectrum direct_illum;
                for(auto light : scene.lights())
                    direct_illum += coef * mis_sample_light(scene, light, scattering_point, phase_function, sampler);
                direct_illum += coef * mis_sample_bsdf(scene, scattering_point, phase_function, sampler);

                ret += direct_illum;

                // sample phase function

                auto bsdf_sample = phase_function->sample(scattering_point.wr, TM_Radiance, sampler.sample3());
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
                    ret += coef * light->radiance(ent_inct, ent_inct.wr);
            }

            // direct illumination

            Spectrum direct_illum;
            for(auto light : scene.lights())
                direct_illum += coef * mis_sample_light(scene, light, ent_inct, ent_shd, sampler);
            direct_illum += coef * mis_sample_bsdf(scene, ent_inct, ent_shd, sampler);

            ret += direct_illum;

            // sample bsdf

            auto bsdf_sample = ent_shd.bsdf->sample(ent_inct.wr, TM_Radiance, sampler.sample3());
            if(!bsdf_sample.f || bsdf_sample.pdf < EPS)
                return ret;

            r = Ray(ent_inct.eps_offset(bsdf_sample.dir), bsdf_sample.dir.normalize());
            coef *= bsdf_sample.f * std::abs(cos(ent_inct.geometry_coord.z, bsdf_sample.dir)) / bsdf_sample.pdf;
        }

        return ret;
    }
};

std::shared_ptr<PathTracingIntegrator> create_mis_integrator(
    int min_depth, int max_depth, real cont_prob)
{
    auto ret = std::make_shared<VolumetricPathTracingIntegrator>();
    ret->initialize(min_depth, max_depth, cont_prob);
    return ret;
}

AGZ_TRACER_END
