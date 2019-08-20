#include <agz/tracer/core/bsdf.h>
#include <agz/tracer/core/intersection.h>
#include <agz/tracer/core/material.h>
#include <agz/tracer/core/path_tracing_integrator.h>
#include <agz/tracer/core/sampler.h>
#include <agz/tracer/core/scene.h>
#include <agz/tracer/core/texture.h>

AGZ_TRACER_BEGIN

class IsolatedPathTracingIntegrator : public PathTracingIntegrator
{
    const Texture *env_tex_ = nullptr;
    const Texture *background_ = nullptr;

    int min_depth_ = 1;
    int max_depth_ = 10;
    real continue_prob_ = real(0.9);

    static Vec2 dir_to_env_uv(const Vec3 &dir) noexcept
    {
        Vec3 ndir = dir.normalize();
        real phi   = local_angle::phi(ndir);
        real theta = local_angle::theta(ndir);
        real u = math::clamp<real>(phi / (2 * PI_r), 0, 1);
        real v = math::clamp<real>(theta / PI_r, 0, 1);
        return { u, v };
    }

public:

    using PathTracingIntegrator::PathTracingIntegrator;

    static std::string description()
    {
        return R"___(
isolated [PathTracingIntegrator]
    env        [Texture] environment texture
    background [Texture] (optional; defaultly set to env) background environment texture
    min_depth [int] minimum path length before applying Russian Roulette
    max_depth [int] maximal path length
    cont_prob [real] continuing probability in Russian Roulette
)___";
    }

    void initialize(const Config &params, obj::ObjectInitContext &init_ctx) override
    {
        AGZ_HIERARCHY_TRY

        env_tex_ = TextureFactory.create(params.child_group("env"), init_ctx);

        if(auto node = params.find_child("background"))
            background_ = TextureFactory.create(node->as_group(), init_ctx);
        else
            background_ = env_tex_;

        min_depth_ = params.child_int("min_depth");
        if(min_depth_ < 1)
            throw ObjectConstructionException("invalid min depth value: " + std::to_string(min_depth_));

        max_depth_ = params.child_int("max_depth");
        if(max_depth_ < min_depth_)
            throw ObjectConstructionException("invalid max depth value: " + std::to_string(max_depth_));

        continue_prob_ = params.child_real("cont_prob");
        if(continue_prob_ < 0 || continue_prob_ > 1)
            throw ObjectConstructionException("invalid continue prob value: " + std::to_string(continue_prob_));

        AGZ_HIERARCHY_WRAP("in initializing isolated path tracing integrator")
    }

    Spectrum eval(GBufferPixel *gpixel, const Scene &scene, const Ray &ray, Sampler &sampler, Arena &arena) const override
    {
        // 寻找primary intersection

        Spectrum coef(1);
        Ray r = ray;
        bool background = true;
        gpixel->depth = -1;

        for(int depth = 1; depth < max_depth_; ++depth)
        {
            if(depth > min_depth_)
            {
                if(sampler.sample1().u > continue_prob_)
                    return {};
                coef /= continue_prob_;
            }

            EntityIntersection inct;
            if(!scene.closest_intersection(r, &inct))
            {
                if(background)
                {
                    Vec2 uv = dir_to_env_uv(r.d);
                    return coef * background_->sample_spectrum(uv);
                }

                Vec2 uv = dir_to_env_uv(r.d);
                Spectrum rad = env_tex_->sample_spectrum(uv);
                return coef * rad;
            }

            auto shd = inct.material->shade(inct, arena);
            if(depth == 1)
            {
                gpixel->albedo   = shd.bsdf->albedo();
                gpixel->position = inct.pos;
                gpixel->depth    = (inct.pos - r.o).length();
                gpixel->normal   = inct.user_coord.z;
            }

            auto bsdf_sample = shd.bsdf->sample(inct.wr, TM_Radiance, sampler.sample3());
            if(!bsdf_sample.f || bsdf_sample.pdf < EPS)
                return {};

            bool is_trans = (dot(inct.wr, inct.geometry_coord.z) > 0) != (dot(bsdf_sample.dir, inct.geometry_coord.z) > 0);
            if(!is_trans)
                background = false;

            coef *= bsdf_sample.f * shd.bsdf->proj_wi_factor(bsdf_sample.dir) / bsdf_sample.pdf;
            r = Ray(inct.pos, bsdf_sample.dir.normalize(), EPS);
        }

        return {};
    }
};

AGZT_IMPLEMENTATION(PathTracingIntegrator, IsolatedPathTracingIntegrator, "isolated")

AGZ_TRACER_END
