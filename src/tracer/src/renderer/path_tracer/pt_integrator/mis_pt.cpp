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

class MISPathTracingIntegrator : public PathTracingIntegrator
{
    int min_depth_ = 1;
    int max_depth_ = 10;
    real cont_prob_ = 1;
    bool sample_all_lights_ = false;

public:

    using PathTracingIntegrator::PathTracingIntegrator;

    static std::string description()
    {
        return R"___(
mis [PathTracingIntegrator]
    min_depth [int]         minimum path length before applying Russian Roulette
    max_depth [int]         maximal path length
    cont_prob [real]        continuing probability in Russian Roulette
    sample_all_lights [0/1] sample all lights in each light sampling iteration
)___";
    }

    void initialize(const Config &params, obj::ObjectInitContext &init_ctx) override
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

        AGZ_HIERARCHY_WRAP("in initializing mis path tracing integrator")
    }

    Spectrum eval(GBufferPixel *gpixel, const Scene &scene, const Ray &r, Sampler &sampler, Arena &arena) const override
    {
        Spectrum ret, coef(1);

        EntityIntersection inct;
        bool has_inct = scene.closest_intersection(r, &inct);
        if(has_inct)
        {
            if(auto light = inct.entity->as_light())
                ret += light->radiance(inct, inct.wr);
        }
        else
            return {};

        for(int i = 1; i <= max_depth_; ++i)
        {
            if(!has_inct)
                break;

            if(i > min_depth_)
            {
                if(sampler.sample1().u > cont_prob_)
                    return ret;
                coef /= cont_prob_;
            }

            auto shd = inct.material->shade(inct, arena);

            if(i == 1)
            {
                gpixel->albedo   = shd.bsdf->albedo();
                gpixel->position = inct.pos;
                gpixel->depth    = r.d.length() * inct.t;
                gpixel->normal   = inct.user_coord.z;
            }

            if(sample_all_lights_)
            {
                for(size_t j = 0; j < scene.light_count(); ++j)
                {
                    auto light = scene.light(j);
                    ret += coef * mis_sample_light(scene, light, inct, shd, sampler.sample5());
                }
            }
            else
            {
                auto [light, pdf] = scene.sample_light(sampler.sample1());
                ret += coef * mis_sample_light(scene, light, inct, shd, sampler.sample5()) / pdf;
            }

            BSDFSampleResult bsdf_sample;
            EntityIntersection new_inct;
            ret += coef * mis_sample_bsdf<true>(scene, inct, shd, sampler.sample3(), &bsdf_sample, &new_inct, &has_inct);
            if(has_inct)
                inct = new_inct;

            coef *= bsdf_sample.f * shd.bsdf->proj_wi_factor(bsdf_sample.dir) / bsdf_sample.pdf;
        }

        return ret;
    }
};

AGZT_IMPLEMENTATION(PathTracingIntegrator, MISPathTracingIntegrator, "mis")

AGZ_TRACER_END
