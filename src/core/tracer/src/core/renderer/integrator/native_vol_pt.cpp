#include <agz/tracer/core/bsdf.h>
#include <agz/tracer/core/entity.h>
#include <agz/tracer/core/light.h>
#include <agz/tracer/core/intersection.h>
#include <agz/tracer/core/material.h>
#include <agz/tracer/core/medium.h>
#include <agz/tracer/core/path_tracing_integrator.h>
#include <agz/tracer/core/sampler.h>
#include <agz/tracer/core/scene.h>
#include <agz/utility/misc.h>

AGZ_TRACER_BEGIN

class NativeVolumetricPathTracingIntegrator : public PathTracingIntegrator
{
    int min_depth_ = 5;
    int max_depth_ = 20;
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

        AGZ_HIERARCHY_WRAP("in initializing volumetric path tracing integrator")
    }

    Spectrum eval(GBufferPixel *gpixel, const Scene &scene, const Ray &ray, Sampler &sampler, Arena &arena) const override
    {
        Spectrum ret, coef(1);
        Ray r = ray;

        for(int depth = 1; depth <= max_depth_; ++depth)
        {
            // RR strategy

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
                for(auto light : scene.nonarea_lights())
                    ret += coef * light->radiance(r.o, r.d);
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

                auto phase_sample = phase_function->sample(ent_inct.wr, TM_Radiance, sampler.sample3());
                if(!phase_sample.f)
                    return ret;

                coef *= phase_sample.f / phase_sample.pdf;
                r = Ray(scattering_point.pos, phase_sample.dir);

                continue;
            }

            // process surface scattering

            if(auto light = ent_inct.entity->as_light())
                ret += coef * light->radiance(ent_inct, ent_inct.wr);

            auto bsdf_sample = ent_shd.bsdf->sample(ent_inct.wr, TM_Radiance, sampler.sample3());
            if(!bsdf_sample.f)
                return ret;

            coef *= bsdf_sample.f * std::abs(cos(ent_inct.geometry_coord.z, bsdf_sample.dir)) / bsdf_sample.pdf;
            r = Ray(ent_inct.eps_offset(bsdf_sample.dir), bsdf_sample.dir);
        }

        return ret;
    }
};

std::shared_ptr<PathTracingIntegrator> create_native_integrator(
    int min_depth, int max_depth, real cont_prob)
{
    auto ret = std::make_shared<NativeVolumetricPathTracingIntegrator>();
    ret->initialize(min_depth, max_depth, cont_prob);
    return ret;
}

AGZ_TRACER_END
