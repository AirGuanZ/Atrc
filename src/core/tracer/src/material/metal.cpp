#include <agz/tracer/core/bsdf.h>
#include <agz/tracer/core/material.h>

#include "./utility/microfacet.h"
#include "agz/tracer/utility/reflection.h"

AGZ_TRACER_BEGIN

namespace metal_impl
{
    
    class MetalBSDF : public LocalBSDF
    {
        Spectrum R0_;
        real roughness_;

        static real pow5(real x) noexcept
        {
            real x2 = x * x;
            return x2 * x2 * x;
        }

    public:

        MetalBSDF(
            const Coord &geometro_coord, const Coord &shading_coord,
            const Spectrum &R0, real roughness) noexcept
            : LocalBSDF(geometro_coord, shading_coord), R0_(R0), roughness_(roughness)
        {
            
        }

        Spectrum eval(const Vec3 &wi, const Vec3 &wo, TransportMode mode) const noexcept override
        {
            if(cause_black_fringes(wi, wo))
                return eval_for_black_fringes(wi, wo);

            Vec3 lwi = shading_coord_.global_to_local(wi).normalize();
            Vec3 lwo = shading_coord_.global_to_local(wo).normalize();
            if(lwi.z <= EPS || lwo.z <= EPS)
                return {};

            Vec3 lwh = (lwi + lwo).normalize();
            real cos_theta_d = dot(lwi, lwh);
            Spectrum F = R0_ + (Spectrum(1) - R0_) * pow5(1 - cos_theta_d);

            real cos_theta_h = local_angle::cos_theta(lwh);
            real D = microfacet::gtr2(cos_theta_h, roughness_);

            real tan_theta_i = local_angle::tan_theta(lwi);
            real tan_theta_o = local_angle::tan_theta(lwo);
            real G = microfacet::smith_gtr2(tan_theta_i, roughness_)
                   * microfacet::smith_gtr2(tan_theta_o, roughness_);

            auto val = F * D * G / (4 * lwi.z * lwo.z);
            return val * local_angle::normal_corr_factor(geometry_coord_, shading_coord_, wi);
        }

        BSDFSampleResult sample(const Vec3 &wo, TransportMode mode, const Sample3 &sam) const noexcept override
        {
            if(cause_black_fringes(wo))
                return sample_for_black_fringes(wo, mode, sam);

            Vec3 lwo = shading_coord_.global_to_local(wo).normalize();
            if(lwo.z <= EPS)
                return BSDF_SAMPLE_RESULT_INVALID;
            
            Vec3 lwh = microfacet::sample_gtr2(roughness_, { sam.u, sam.v });
            if(lwh.z <= 0)
                return BSDF_SAMPLE_RESULT_INVALID;

            Vec3 lwi = refl_aux::reflect(lwo, lwh).normalize();
            if(lwi.z <= EPS)
                return BSDF_SAMPLE_RESULT_INVALID;
            Vec3 wi = shading_coord_.local_to_global(lwi);

            Spectrum f;
            real pdf;
            {
                real cos_theta_d = dot(lwi, lwh);
                Spectrum F = R0_ + (Spectrum(1) - R0_) * pow5(1 - cos_theta_d);

                real cos_theta_h = local_angle::cos_theta(lwh);
                real D = microfacet::gtr2(cos_theta_h, roughness_);

                real tan_theta_i = local_angle::tan_theta(lwi);
                real tan_theta_o = local_angle::tan_theta(lwo);
                real G = microfacet::smith_gtr2(tan_theta_i, roughness_)
                       * microfacet::smith_gtr2(tan_theta_o, roughness_);

                auto val = F * D * G / (4 * lwi.z * lwo.z);
                f = val * local_angle::normal_corr_factor(geometry_coord_, shading_coord_, wi);

                pdf = cos_theta_h * D / (4 * cos_theta_d);
            }

            BSDFSampleResult ret;
            ret.dir      = wi;
            ret.f        = f;
            ret.pdf      = pdf;
            ret.mode     = mode;
            ret.is_delta = false;

            return ret;
        }

        real pdf(const Vec3 &wi, const Vec3 &wo, TransportMode mode) const noexcept override
        {
            if(cause_black_fringes(wi, wo))
                return pdf_for_black_fringes(wi, wo);

            Vec3 lwi = shading_coord_.global_to_local(wi).normalize();
            Vec3 lwo = shading_coord_.global_to_local(wo).normalize();
            if(lwi.z <= EPS || lwo.z <= EPS)
                return 0;

            Vec3 lwh = (lwi + lwo).normalize();
            real cos_theta_h = local_angle::cos_theta(lwh);
            real cos_theta_d = dot(lwi, lwh);
            real D = microfacet::gtr2(lwh.z, roughness_);

            return cos_theta_h * D / (4 * cos_theta_d);
        }

        Spectrum albedo() const noexcept override
        {
            return R0_;
        }

        bool is_black() const noexcept override
        {
            return false;
        }
    };

} // namespace metal_impl

class Metal : public Material
{
    Spectrum R0_;
    real roughness_ = real(0.1);

public:

    static std::string description()
    {
        return R"___(
metal [Material]
    R0        [Spectrum] R0 in schlick fresnel formula
    roughness [real]     metal roughness in (0, 1]
)___";
    }

    void initialize(const Config &params, obj::ObjectInitContext&) override
    {
        AGZ_HIERARCHY_TRY

        R0_ = params.child_spectrum("R0");
        roughness_ = params.child_real("roughness");

        AGZ_HIERARCHY_WRAP("in initializing metal material")
    }

    ShadingPoint shade(const EntityIntersection &inct, Arena &arena) const override
    {
        auto bsdf = arena.create<metal_impl::MetalBSDF>(
            inct.geometry_coord, inct.user_coord, R0_, roughness_);
        return { bsdf };
    }
};

AGZT_IMPLEMENTATION(Material, Metal, "metal")

AGZ_TRACER_END
