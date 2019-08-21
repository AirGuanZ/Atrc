#include <agz/tracer/core/bsdf.h>
#include <agz/tracer/core/entity.h>
#include <agz/tracer/core/light.h>
#include <agz/tracer/core/intersection.h>
#include <agz/tracer/core/material.h>
#include <agz/tracer/core/path_tracing_integrator.h>
#include <agz/tracer/core/sampler.h>
#include <agz/tracer/core/scene.h>

AGZ_TRACER_BEGIN

class NativePathTracingIntegrator : public PathTracingIntegrator
{
    int min_depth_ = 1;
    int max_depth_ = 10;
    real continue_prob_ = real(0.9);

public:

    using PathTracingIntegrator::PathTracingIntegrator;

    static std::string description()
    {
        return R"___(
native [PathTracingIntegrator]
    min_depth [int] minimum path length before applying Russian Roulette
    max_depth [int] maximal path length
    cont_prob [real] continuing probability in Russian Roulette
)___";
    }

    void initialize(const Config &params, obj::ObjectInitContext &) override
    {
        AGZ_HIERARCHY_TRY

        min_depth_ = params.child_int("min_depth");
        if(min_depth_ < 1)
            throw ObjectConstructionException("invalid min depth value: " + std::to_string(min_depth_));

        max_depth_ = params.child_int("max_depth");
        if(max_depth_ < min_depth_)
            throw ObjectConstructionException("invalid max depth value: " + std::to_string(max_depth_));
        
        continue_prob_ = params.child_real("cont_prob");
        if(continue_prob_ < 0 || continue_prob_ > 1)
            throw ObjectConstructionException("invalid continue prob value: " + std::to_string(continue_prob_));

        AGZ_HIERARCHY_WRAP("in initializing native path tracing integrator")
    }

    Spectrum eval(GBufferPixel *gpixel, const Scene &scene, const Ray &r, Sampler &sampler, Arena &arena) const override
    {
        Spectrum ret, coef(1);
        Ray ray = r;

        for(int i = 1; i < max_depth_; ++i)
        {
            if(i > min_depth_)
            {
                if(sampler.sample1().u > continue_prob_)
                    return ret;
                coef /= continue_prob_;
            }

            EntityIntersection inct;
            if(!scene.closest_intersection(ray, &inct))
                return ret;

            if(auto area_light = inct.entity->as_light())
                ret += coef * area_light->radiance(inct, inct.wr);

            ShadingPoint shd = inct.material->shade(inct, arena);
            if(i == 1)
            {
                gpixel->albedo   = shd.bsdf->albedo();
                gpixel->position = inct.pos;
                gpixel->depth    = r.d.length() * inct.t;
                gpixel->normal   = inct.user_coord.z;
            }

            auto bsdf_sample = shd.bsdf->sample(inct.wr, TM_Radiance, sampler.sample3());
            if(!bsdf_sample.f)
                return ret;

            coef *= bsdf_sample.f * std::abs(cos(bsdf_sample.dir, inct.geometry_coord.z)) / bsdf_sample.pdf;

            Vec3 true_nor = inct.geometry_coord.z;
            if(dot(bsdf_sample.dir, inct.geometry_coord.z) < 0)
                true_nor = -true_nor;
            Vec3 new_pos = inct.pos + EPS * true_nor;
            ray = Ray(new_pos, bsdf_sample.dir.normalize());
        }

        return ret;
    }
};

AGZT_IMPLEMENTATION(PathTracingIntegrator, NativePathTracingIntegrator, "native")

AGZ_TRACER_END
