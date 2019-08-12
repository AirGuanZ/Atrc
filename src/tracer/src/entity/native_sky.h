#pragma once

#include "./infinite_light.h"

AGZ_TRACER_BEGIN

class NativeSkyLightImpl : public InfiniteLightImpl
{
    Spectrum top_, bottom_;
    Vec3 up_dir_;

    Spectrum radiance(const Vec3 &ref_to_light) const noexcept
    {
        real cos_theta = math::clamp<real>(dot(ref_to_light.normalize(), up_dir_), -1, 1);
        real s = (cos_theta + 1) / 2;
        return s * top_ + (1 - s) * bottom_;
    }

public:

    static std::string description()
    {
        return R"___(
native_sky [(Nonarea)Light]
    top    [Spectrum] top radiance
    bottom [Spectrum] bottom radiance
    up     [Vec3]     up direction
)___";
    }

    void initialize(const Config &params, obj::ObjectInitContext&) override
    {
        AGZ_HIERARCHY_TRY

        top_ = params.child_spectrum("top");
        bottom_ = params.child_spectrum("bottom");

        if(params.find_child("up"))
            up_dir_ = params.child_vec3("up").normalize();
        else
            up_dir_ = Vec3(0, 0, 1);

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

    real pdf(const InfiniteLightCore &, const Vec3&, const Vec3 &) const noexcept override
    {
        return math::distribution::uniform_on_sphere_pdf<real>;
    }

    Spectrum power(const InfiniteLightCore &core) const noexcept override
    {
        real radius = core.world_radius();
        Spectrum mean_radiance = (top_ + bottom_) * real(0.5);
        return 4 * PI_r * PI_r * radius * radius * mean_radiance;
    }

    Spectrum radiance(const InfiniteLightCore &core, const Vec3 &ref_to_light) const noexcept override
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

