#pragma once

#include <random>

#include <agz/tracer/core/texture.h>
#include "./infinite_light.h"

AGZ_TRACER_BEGIN

class EnvironmentLightImpl : public InfiniteLightImpl
{
    const Texture *tex_ = nullptr;

    Spectrum radiance(const Vec3 &ref_to_light) const noexcept
    {
        Vec3 dir = ref_to_light.normalize();
        real phi = local_angle::phi(dir);
        real theta = local_angle::theta(dir);
        real u = math::clamp<real>(phi / (2 * PI_r), 0, 1);
        real v = math::clamp<real>(theta / PI_r, 0, 1);
        return tex_->sample_spectrum({ u, v });
    }

public:

    static std::string description()
    {
        return R"___(
env [(Nonarea)Light]
    tex [Spectrum] environment texture
)___";
    }

    void initialize(const Config &params, obj::ObjectInitContext &init_ctx) override
    {
        AGZ_HIERARCHY_TRY

        tex_ = TextureFactory.create(params.child_group("tex"), init_ctx);

        AGZ_HIERARCHY_WRAP("in initializing native sky light")
    }

    LightSampleResult sample(const InfiniteLightCore &core, const Vec3 &ref, const Sample4 &sam) const noexcept override
    {
        auto [dir, pdf] = math::distribution::uniform_on_sphere(sam.u, sam.v);

        Ray global_r(ref, dir, EPS);
        GeometryIntersection inct;
        core.intersection(global_r, &inct);

        LightSampleResult ret;
        ret.spt = static_cast<SurfacePoint&>(inct);
        ret.radiance = radiance(dir);
        ret.pdf      = pdf;
        ret.is_delta = false;

        return ret;
    }

    real pdf(const InfiniteLightCore &core, const Vec3&, const Vec3 &) const noexcept override
    {
        return math::distribution::uniform_on_sphere_pdf<real>;
    }

    Spectrum power(const InfiniteLightCore &core) const noexcept override
    {
        real radius = core.world_radius();

        Spectrum mean_radiance;
        std::default_random_engine rng;
        std::uniform_real_distribution<real> dis(0, 1);

        constexpr int N = 10000;
        for(int i = 0; i < N; ++i)
        {
            real u1 = dis(rng), u2 = dis(rng);
            auto[dir, pdf] = math::distribution::uniform_on_sphere(u1, u2);
            mean_radiance += radiance(dir);
        }
        mean_radiance /= N;

        return 4 * PI_r * PI_r * radius * radius * mean_radiance;
    }

    Spectrum radiance(const InfiniteLightCore &, const Vec3 &ref_to_light) const noexcept override
    {
        return radiance(ref_to_light);
    }
    
    LightEmitResult emit(const InfiniteLightCore &core, const Sample4 &sam) const noexcept override
    {
        LightEmitResult ret;

        auto [unit_pos, unit_pos_pdf] = math::distribution::uniform_on_sphere(sam.u, sam.v);
        ret.spt = core.unit_pos_to_spt(unit_pos);
        ret.pdf_pos = unit_pos_pdf / (core.world_radius() * core.world_radius());

        auto [local_dir, dir_pdf] = math::distribution::zweighted_on_hemisphere(sam.w, sam.r);
        auto global_dir = ret.spt.geometry_coord.local_to_global(local_dir);
        ret.dir = global_dir;
        ret.pdf_dir = dir_pdf;
        
        ret.radiance = radiance(-global_dir);

        return ret;
    }

    void emit_pdf(const InfiniteLightCore &core, const SurfacePoint &spt, const Vec3 &light_to_out,
                  real *pdf_pos, real *pdf_dir) const noexcept override
    {
        real z = cos(light_to_out, spt.geometry_coord.z);
        *pdf_pos = math::distribution::uniform_on_sphere_pdf<real> / (core.world_radius() * core.world_radius());
        *pdf_dir = math::distribution::zweighted_on_hemisphere_pdf(z);
    }
};

AGZ_TRACER_END

