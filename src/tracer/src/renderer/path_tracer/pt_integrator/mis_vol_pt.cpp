#include <agz/tracer/core/bsdf.h>
#include <agz/tracer/core/entity.h>
#include <agz/tracer/core/light.h>
#include <agz/tracer/core/intersection.h>
#include <agz/tracer/core/material.h>
#include <agz/tracer/core/path_tracing_integrator.h>
#include <agz/tracer/core/sampler.h>
#include <agz/tracer/core/scene.h>

#include "./mis_light_bsdf.h"

AGZ_TRACER_BEGIN

class VolumetricPathTracingIntegrator : public PathTracingIntegrator
{
    int min_depth_ = 5;
    int max_depth_ = 20;
    real cont_prob_ = real(0.5);
    bool sample_all_lights_ = true;

public:

    static std::string description()
    {
        return R"___(
mis_vol [PathTracingIntegrator]
    min_depth [int]         minimum path length before applying Russian Roulette
    max_depth [int]         maximal path length
    cont_prob [real]        continuing probability in Russian Roulette
    sample_all_lights [0/1] sample all lights in each light sampling iteration
)___";
    }

    void initialize(const Config &params, obj::ObjectInitContext&) override
    {
        AGZ_HIERARCHY_TRY

        min_depth_ = params.child_int("min_depth");
        if(min_depth_ < 1)
            throw ObjectConstructionException("invalid min depth value: " + std::to_string(min_depth_));

        max_depth_ = params.child_int("max_depth");
        if(max_depth_ < min_depth_)
            throw ObjectConstructionException("invalid max depth value: " + std::to_string(max_depth_));

        cont_prob_ = params.child_real("cont_prob");
        if(cont_prob_ < 0 || cont_prob_ > 1)
            throw ObjectConstructionException("invalid continue prob value: " + std::to_string(cont_prob_));

        sample_all_lights_ = params.child_int("sample_all_lights") != 0;

        AGZ_HIERARCHY_WRAP("in initializing volumetric path tracing integrator")
    }

    Spectrum eval(GBufferPixel *gpixel, const Scene &scene, const Ray &ray, Sampler &sampler, Arena &arena) const override
    {
        Spectrum ret, coef(1);
        Ray r = ray;

        for(int depth = 1; depth <= max_depth_; ++depth)
        {
            if(depth > min_depth_)
            {
                if(sampler.sample1().u > cont_prob_)
                    return ret;
                coef /= cont_prob_;
            }

            bool has_ent_inct;  EntityIntersection ent_inct;
            SampleScatteringResult scattering_sample;
            scattering_sample.p_has_inct = &has_ent_inct;
            scattering_sample.p_inct     = &ent_inct;
            if(!scene.next_scattering_point(r, &scattering_sample, sampler.sample1(), arena))
                return ret;
            auto &pnt = scattering_sample.pnt;

            auto medium = pnt.medium(pnt.wr());
            Spectrum tr = medium->tr(r.o, pnt.pos());
            coef *= tr / scattering_sample.pdf;

            if(depth == 1)
            {
                if(has_ent_inct)
                {
                    if(pnt.is_on_surface())
                        gpixel->albedo = pnt.bsdf()->albedo();
                    else
                        gpixel->albedo = ent_inct.material->shade(ent_inct, arena).bsdf->albedo();
                    gpixel->position = ent_inct.pos;
                    gpixel->normal = ent_inct.user_coord.z;
                    gpixel->depth = r.d.length() * ent_inct.t;
                }

                if(pnt.is_on_surface())
                {
                    auto inct = pnt.as_entity_inct();
                    if(auto light = inct.entity->as_light())
                        ret += coef * light->radiance(inct, inct.wr);
                }
            }

            if(sample_all_lights_)
            {
                for(auto light : scene.lights())
                    ret += coef * mis_sample_light(scene, light, pnt, sampler.sample5());
            }
            else
            {
                auto [light, pdf] = scene.sample_light(sampler.sample1());
                ret += coef * mis_sample_light(scene, light, pnt, sampler.sample5()) / pdf;
            }

            ret += coef * mis_sample_scattering(scene, pnt, sampler.sample3());

            BSDFSampleResult bsdf_sample = pnt.sample(pnt.wr(), sampler.sample3(), TM_Radiance);
            if(!bsdf_sample.f || bsdf_sample.pdf < EPS)
                break;

            r = Ray(pnt.pos(), bsdf_sample.dir.normalize(), EPS);
            coef *= bsdf_sample.f * pnt.proj_wi_factor(bsdf_sample.dir) / bsdf_sample.pdf;
        }

        return ret;
    }
};

AGZT_IMPLEMENTATION(PathTracingIntegrator, VolumetricPathTracingIntegrator, "mis_vol");

AGZ_TRACER_END
