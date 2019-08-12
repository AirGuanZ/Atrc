#pragma once

#include "./infinite_light.h"

AGZ_TRACER_BEGIN

class DirectionalLightImpl : public InfiniteLightImpl
{
    // local light cone总是从+z方向向-z方向照射
    Coord local_cone_space_;
    Spectrum radiance_;
    real max_cos_theta_ = real(0.5);

    Spectrum radiance(const Vec3 &ref_to_light) const noexcept
    {
        Vec3 local_dir = local_cone_space_.global_to_local(ref_to_light);
        real cos_theta = local_angle::cos_theta(local_dir.normalize());
        if(cos_theta >= max_cos_theta_)
            return radiance_;
        return {};
    }

    static Vec2 dir_to_uv(const Vec3 &d)
    {
        Vec3 dir = d.normalize();
        real phi = local_angle::phi(dir);
        real theta = local_angle::theta(dir);
        real u = math::clamp<real>(phi / (2 * PI_r), 0, 1);
        real v = math::clamp<real>(theta / PI_r, 0, 1);
        return { u, v };
    }

public:

    static std::string description()
    {
        return R"___(
dir [(Nonarea)Light]
    dir      [Vec3] radiance main direction
    radiance [Spectrum] radiance value
    range    [real] direction cone size; must be in (0, 1)
)___";
    }

    void initialize(const Config &params, obj::ObjectInitContext &init_ctx) override
    {
        AGZ_HIERARCHY_TRY

        Vec3 dir = params.child_vec3("dir");

        local_cone_space_ = Coord::from_z(-dir);
        radiance_ = params.child_spectrum("radiance");

        max_cos_theta_ = 1 - params.child_real("range");
        if(max_cos_theta_ <= 0 || max_cos_theta_ >= 1)
            throw ObjectConstructionException("invalid range value");

        AGZ_HIERARCHY_WRAP("in initializing directional light object")
    }

    LightSampleResult sample(const InfiniteLightCore &core, const Vec3 &ref, const Sample4 &sam) const noexcept override
    {
        auto [local_dir, pdf] = math::distribution::uniform_on_cone(max_cos_theta_, sam.u, sam.v);
        Vec3 global_dir = local_cone_space_.local_to_global(local_dir).normalize();

        Ray global_r(ref, global_dir, EPS);
        GeometryIntersection inct;
        core.intersection(global_r, &inct);

        LightSampleResult ret;
        ret.spt      = static_cast<SurfacePoint&>(inct);
        ret.radiance = radiance(global_dir);
        ret.pdf      = pdf;
        ret.is_delta = false;

        return ret;
    }

    real pdf(const InfiniteLightCore &, const Vec3 &ref, const Vec3 &ref_to_light) const noexcept override
    {
        Vec3 local_dir = local_cone_space_.global_to_local(ref_to_light);
        real cos_theta = local_angle::cos_theta(local_dir.normalize());
        if(cos_theta >= max_cos_theta_)
            return math::distribution::uniform_on_cone_pdf(max_cos_theta_);
        return 0;
    }

    Spectrum power(const InfiniteLightCore &core) const noexcept override
    {
        real radius = core.world_radius();
        return 4 * PI_r * PI_r * radius * radius * ((1 - max_cos_theta_) / 2 * radiance_);
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

        auto [local_dir, dir_pdf] = math::distribution::uniform_on_cone(max_cos_theta_, sam.w, sam.r);
        auto global_dir = -local_cone_space_.local_to_global(local_dir);
        ret.dir = global_dir;
        ret.pdf_dir = dir_pdf;
        
        ret.radiance = radiance(-global_dir);

        return ret;
    }

    void emit_pdf(const InfiniteLightCore &core, const SurfacePoint &spt, const Vec3 &light_to_out,
                  real *pdf_pos, real *pdf_dir) const noexcept override
    {
        *pdf_pos = math::distribution::uniform_on_sphere_pdf<real> / (core.world_radius() * core.world_radius());
        Vec3 global_ref_to_light = -light_to_out;
        Vec3 local_ref_to_light = local_cone_space_.global_to_local(global_ref_to_light);
        real cos_theta = local_ref_to_light.normalize().z;
        if(cos_theta >= max_cos_theta_)
            *pdf_dir = math::distribution::uniform_on_cone_pdf(max_cos_theta_);
        else
            *pdf_dir = 0;
    }
};

AGZ_TRACER_END
