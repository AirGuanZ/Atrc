#include <agz/tracer/core/bsdf.h>
#include <agz/tracer/core/entity.h>
#include <agz/tracer/core/guided_pt_integrator.h>
#include <agz/tracer/core/intersection.h>
#include <agz/tracer/core/light.h>
#include <agz/tracer/core/material.h>

#include "../guider.h"

AGZ_TRACER_BEGIN

using namespace pgpt;

class NativeGuidedPathTracingIntegrator : public GuidedPathTracingIntegrator
{
    int min_depth_ = 5;
    int max_depth_ = 10;
    real cont_prob_ = real(0.9);

    template<bool Commit>
    Spectrum integrate(
        const Scene& scene, const Ray &r, Sampler &sampler, Arena &arena,
        RecordBatchBuilder &recorder, Guider& guider, bool enable_trainer, bool enable_sampler, int depth) const
    {
        bool allow_commit = Commit && enable_trainer;
        real rr_factor = 1;
        auto commit = [&recorder, &r, allow_commit, &rr_factor]
            (const Spectrum &radiance)
        {
            Spectrum ret = rr_factor * radiance;
            if(allow_commit)
                recorder.add({ r.o, r.d, ret });
            return ret;
        };

        if(depth >= min_depth_)
        {
            if(depth > max_depth_)
                return Spectrum();
            if(sampler.sample1().u > cont_prob_)
                return commit(Spectrum());
            rr_factor /= cont_prob_;
        }

        Spectrum ret;

        EntityIntersection inct;
        if(!scene.closest_intersection(r, &inct))
            return Spectrum();

        if(auto light = inct.entity->as_light())
            ret += light->radiance(inct, inct.wr);

        auto shd = inct.material->shade(inct, arena);
        auto bsdf = shd.bsdf;

        bool sample_guider = enable_sampler && !bsdf->is_delta();
        real sample_guider_prob = sample_guider ? real(0.5) : 0;

        Vec3 new_dir; Spectrum new_f;

        if(sample_guider && sampler.sample1().u < sample_guider_prob)
        {
            auto [dir, guider_pdf] = guider.dsampler(inct.pos)->sample(sampler.sample4());
            new_dir = dir.normalize();

            new_f = bsdf->eval(new_dir, inct.wr, TM_Radiance) * bsdf->proj_wi_factor(new_dir);
            if(!new_f)
                return commit(ret);

            real bsdf_pdf = bsdf->pdf(new_dir, inct.wr, TM_Radiance);
            new_f /= (guider_pdf + bsdf_pdf) * sample_guider_prob;
        }
        else
        {
            real sample_bsdf_prob = 1 - sample_guider_prob;

            auto bsdf_sample = bsdf->sample(inct.wr, TM_Radiance, sampler.sample3());
            if(!bsdf_sample.f || bsdf_sample.pdf < EPS)
                return commit(ret);

            new_dir = bsdf_sample.dir.normalize();
            new_f = bsdf_sample.f * bsdf->proj_wi_factor(new_dir);

            real guider_pdf = sample_guider && !bsdf_sample.is_delta ? guider.dsampler(inct.pos)->pdf(new_dir) : 0;
            new_f /= (guider_pdf + bsdf_sample.pdf) * sample_bsdf_prob;
        }

        Vec3 true_nor = inct.geometry_coord.z;
        if(dot(new_dir, inct.geometry_coord.z) < 0)
            true_nor = -true_nor;
        Vec3 new_pos = inct.pos + EPS * true_nor;
        Ray new_ray(new_pos, new_dir.normalize());

        ret += new_f * integrate<true>(
            scene, new_ray, sampler, arena,
            recorder, guider, enable_trainer, enable_sampler, depth + 1);

        return commit(ret);
    }

public:

    using GuidedPathTracingIntegrator::GuidedPathTracingIntegrator;

    static std::string description()
    {
        return R"___(
native [GuidedPathTracingIntegrator]
    min_depth [int]         minimum path length before applying Russian Roulette
    max_depth [int]         maximal path length
    cont_prob [real]        continuing probability in Russian Roulette
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

        AGZ_HIERARCHY_WRAP("in initializing native guided path tracing integrator")
    }

    Spectrum eval(
        const Scene& scene, const Ray &r, Sampler &sampler, Arena &arena,
        RecordBatchBuilder &recorder, Guider& guider, bool enable_trainer, bool enable_sampler) const override
    {
        return integrate<false>(scene, r, sampler, arena, recorder, guider, enable_trainer, enable_sampler, 1);
    }
};

AGZT_IMPLEMENTATION(GuidedPathTracingIntegrator, NativeGuidedPathTracingIntegrator, "native")

AGZ_TRACER_END
