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

class DirectPathTracingIntegrator : public PathTracingIntegrator
{
    bool sample_all_lights_ = false;

public:

    using PathTracingIntegrator::PathTracingIntegrator;

    static std::string description()
    {
        return R"___(
direct [PathTracingIntegrator]
    sample_all_lights [0/1] sample all lights in each light sampling iteration or not
)___";
    }

    void initialize(const Config &params, obj::ObjectInitContext&) override
    {
        AGZ_HIERARCHY_TRY

        sample_all_lights_ = params.child_int("sample_all_lights") != 0;

        AGZ_HIERARCHY_WRAP("in initializing direct path tracing integrator")
    }

    Spectrum eval(GBufferPixel *gpixel, const Scene &scene, const Ray &r, Sampler &sampler, Arena &arena) const override
    {
        EntityIntersection inct;
        if(!scene.closest_intersection(r, &inct))
            return {};

        Spectrum ret;
        if(auto light = inct.entity->as_light())
            ret += light->radiance(inct, inct.wr);

        auto shd = inct.material->shade(inct, arena);

        gpixel->albedo   = shd.bsdf->albedo();
        gpixel->position = inct.pos;
        gpixel->depth    = r.d.length() * inct.t;
        gpixel->normal   = inct.user_coord.z;
        
        if(sample_all_lights_)
        {
            for(size_t i = 0; i < scene.light_count(); ++i)
            {
                auto light = scene.light(i);
                ret += mis_sample_light(scene, light, inct, shd, sampler.sample5());
            }
        }
        else
        {
            auto [light, pdf] = scene.sample_light(sampler.sample1());
            ret += mis_sample_light(scene, light, inct, shd, sampler.sample5()) / pdf;
        }

        ret += mis_sample_bsdf(scene, inct, shd, sampler.sample3());

        return ret;
    }
};

AGZT_IMPLEMENTATION(PathTracingIntegrator, DirectPathTracingIntegrator, "direct")

AGZ_TRACER_END
