#include <agz/tracer/core/bsdf.h>
#include <agz/tracer/core/entity.h>
#include <agz/tracer/core/guided_pt_integrator.h>
#include <agz/tracer/core/intersection.h>
#include <agz/tracer/core/light.h>
#include <agz/tracer/core/material.h>

#include "../../path_tracer/pt_integrator/mis_light_bsdf.h"
#include "../guider.h"

AGZ_TRACER_BEGIN

class MISGuidedPathTracingIntegrator : public GuidedPathTracingIntegrator
{
    int min_depth_  = 5;
    int max_depth_  = 20;
    real cont_prob_ = real(0.9);

    bool sample_all_lights_ = true;

    template<bool Commit>
    static void commit_aux(const Spectrum &radiance, const Ray &ray, pgpt::RecordBatchBuilder &path_recorder)
    {
        if constexpr(Commit)
            path_recorder.add({ ray.o, ray.d, radiance });
    }

    // 设某射线与场景交于inct，求“从inct往-inct.incident_direction的radiance - inct朝这个方向的自发光”
    template<bool Commit>
    Spectrum integrate_excluding_source(
        const Vec3 &ray_o, const Scene &scene, const EntityIntersection &inct, Sampler &sampler, Arena &arena,
        pgpt::RecordBatchBuilder &recorder, pgpt::Guider &guider,
        bool enable_trainer, bool enable_sampler, int depth) const
    {
        bool allow_commit = Commit && enable_trainer;

        real rr_factor = 1;
        auto commit = [&inct, &recorder, &ray_o, allow_commit, &rr_factor]
            (const Spectrum &radiance)
        {
            Spectrum ret = rr_factor * radiance;
            if(allow_commit)
                recorder.add({ ray_o, -inct.wr, ret });
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

        auto shd = inct.material->shade(inct, arena);
        auto bsdf = shd.bsdf;

        // mis bsdf & light，计算直接光照

        if(!bsdf->is_delta())
        {
            if(sample_all_lights_)
            {
                for(size_t i = 0; i < scene.light_count(); ++i)
                    ret += mis_sample_light(scene, scene.light(i), inct, shd, sampler.sample5());
            }
            else
            {
                auto[light, select_light_pdf] = scene.sample_light(sampler.sample1());
                ret += mis_sample_light(scene, light, inct, shd, sampler.sample5()) / select_light_pdf;
            }
        }

        BSDFSampleResult bsdf_sample;
        EntityIntersection new_inct;
        bool has_new_inct;
        ret += mis_sample_bsdf<true>(scene, inct, shd, sampler.sample3(), &bsdf_sample, &new_inct, &has_new_inct);

        // mis guider & bsdf，构造新ray计算间接光照

        constexpr real RAW_SAMPLE_BSDF_PROB   = real(0.5);
        constexpr real RAW_SAMPLE_GUIDER_PROB = real(0.5);

        bool sample_guider = enable_sampler && !bsdf->is_delta();
        real sample_bsdf_prob   = RAW_SAMPLE_BSDF_PROB;
        real sample_guider_prob = sample_guider ? RAW_SAMPLE_GUIDER_PROB : 0;
        {
            real ratio = 1 / (sample_bsdf_prob + sample_guider_prob);
            sample_bsdf_prob   *= ratio;
            sample_guider_prob *= ratio;
        }

        Spectrum new_f; Vec3 new_dir;

        real sample_method_selector = sampler.sample1().u;
        if(sample_guider && sample_method_selector < sample_guider_prob)
        {
            // sample guider

            auto [dir, guider_pdf] = guider.dsampler(inct.pos)->sample(sampler.sample4());
            new_dir = dir.normalize();

            new_f = bsdf->eval(new_dir, inct.wr, TM_Radiance) * bsdf->proj_wi_factor(new_dir);
            if(!new_f)
                return commit(ret);

            real bsdf_pdf = bsdf->pdf(new_dir, inct.wr, TM_Radiance);
            new_f /= (bsdf_pdf + guider_pdf) * sample_guider_prob;

            Vec3 true_nor = inct.geometry_coord.z;
            if(dot(new_dir, inct.geometry_coord.z) < 0)
                true_nor = -true_nor;
            Vec3 new_pos = inct.pos + EPS * true_nor;
            Ray new_ray(new_pos, new_dir.normalize());

            has_new_inct = scene.closest_intersection(new_ray, &new_inct);
        }
        else
        {
            // sample bsdf
            // simply reuse bsdf_sample, new_inct and has_new_inct

            if(!bsdf_sample.f || bsdf_sample.pdf < EPS)
                return commit(ret);

            new_dir = bsdf_sample.dir.normalize();
            new_f = bsdf_sample.f * bsdf->proj_wi_factor(new_dir);

            real guider_pdf = !bsdf_sample.is_delta && sample_guider ? guider.dsampler(inct.pos)->pdf(new_dir) : 0;
            new_f /= (bsdf_sample.pdf + guider_pdf) * sample_bsdf_prob;
        }

        if(has_new_inct)
        {
            ret += new_f * integrate_excluding_source<true>(
                inct.pos, scene, new_inct, sampler, arena,
                recorder, guider, enable_trainer, enable_sampler, depth + 1);
        }

        return commit(ret);
    }

public:

    using GuidedPathTracingIntegrator::GuidedPathTracingIntegrator;

    static std::string description()
    {
        return R"___(
mis [GuidedPathTracingIntegrator]
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

        AGZ_HIERARCHY_WRAP("in initializing mis guided path tracing integrator")
    }

    Spectrum eval(
        const Scene &scene, const Ray &r, Sampler &sampler, Arena &arena,
        pgpt::RecordBatchBuilder &recorder, pgpt::Guider &guider, bool enable_trainer, bool enable_sampler) const override
    {
        Spectrum ret;

        EntityIntersection inct;
        if(!scene.closest_intersection(r, &inct))
            return Spectrum();

        if(auto light = inct.entity->as_light())
            ret += light->radiance(inct, inct.wr);

        ret += integrate_excluding_source<false>(
            r.o, scene, inct, sampler, arena,
            recorder, guider, enable_trainer, enable_sampler, 1);
        return ret;
    }
};

AGZT_IMPLEMENTATION(GuidedPathTracingIntegrator, MISGuidedPathTracingIntegrator, "mis")

AGZ_TRACER_END
