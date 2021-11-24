#include <optional>

#include <agz/tracer/core/bsdf.h>
#include <agz/tracer/core/bssrdf.h>
#include <agz/tracer/core/material.h>
#include <agz/tracer/core/texture2d.h>
#include <agz-utils/misc.h>

#include "./utility/fresnel_point.h"

AGZ_TRACER_BEGIN

namespace
{
    class GlassBSDF : public LocalBSDF
    {
        const DielectricFresnelPoint *fresnel_point_;
        FSpectrum color_reflection_;
        FSpectrum color_refraction_;

        static std::optional<FVec3> refr_dir(
            const FVec3 &nwo, const FVec3 &nor, real eta)
        {
            const real cos_theta_i = std::abs(nwo.z);
            const real sin_theta_i_2 = (std::max)(real(0), 1 - cos_theta_i * cos_theta_i);
            const real sin_theta_t_2 = eta * eta * sin_theta_i_2;
            if(sin_theta_t_2 >= 1)
                return std::nullopt;
            const real cosThetaT = std::sqrt(1 - sin_theta_t_2);
            return (eta * cos_theta_i - cosThetaT) * nor - eta * nwo;
        }

    public:

        GlassBSDF(const FCoord &geometry_coord, const FCoord &shading_coord,
                  const DielectricFresnelPoint *fresnel_point,
                  const FSpectrum &color_reflection,
                  const FSpectrum &color_refraction) noexcept
            : LocalBSDF(geometry_coord, shading_coord),
              fresnel_point_(fresnel_point),
              color_reflection_(color_reflection),
              color_refraction_(color_refraction)
        {

        }

        FSpectrum eval(
            const FVec3 &, const FVec3 &, TransMode, uint8_t type) const noexcept override
        {
            return FSpectrum();
        }

        BSDFSampleResult sample(
            const FVec3 &out_dir, TransMode transport_mode,
            const Sample3 &sam, uint8_t type) const noexcept override
        {
            if(!(type & BSDF_SPECULAR))
                return BSDF_SAMPLE_RESULT_INVALID;

            const FVec3 nwo = shading_coord_.global_to_local(out_dir).normalize();
            const FVec3 nor = nwo.z > 0 ? FVec3(0, 0, 1) : FVec3(0, 0, -1);

            const FSpectrum fr = fresnel_point_->eval(nwo.z);
            if(sam.u < fr.r)
            {
                const FVec3 lwi = FVec3(-nwo.x, -nwo.y, nwo.z);

                const FVec3 wi = shading_coord_.local_to_global(lwi);
                const FSpectrum f = color_reflection_ * fr / std::abs(lwi.z);
                const real norm_factor = local_angle::normal_corr_factor(
                    geometry_coord_, shading_coord_, wi);

                return BSDFSampleResult(wi, f * norm_factor, fr.r, true);
            }

            const real eta_i = nwo.z > 0 ? fresnel_point_->eta_o()
                                         : fresnel_point_->eta_i();
            const real eta_t = nwo.z > 0 ? fresnel_point_->eta_i()
                                         : fresnel_point_->eta_o();
            const real eta = eta_i / eta_t;

            auto opt_wi = refr_dir(nwo, nor, eta);
            if(!opt_wi)
                return BSDF_SAMPLE_RESULT_INVALID;
            const FVec3 nwi = opt_wi->normalize();

            const real corr_factor = transport_mode == TransMode::Radiance ?
                                     eta * eta : real(1);

            const FVec3 dir = shading_coord_.local_to_global(nwi);
            const FSpectrum f = corr_factor * color_refraction_
                             * (1 - fr.r) / std::abs(nwi.z);
            const real pdf = 1 - fr.r;
            const real norm_factor = local_angle::normal_corr_factor(
                geometry_coord_, shading_coord_, dir);

            return BSDFSampleResult(dir, f * norm_factor, pdf, true);
        }

        real pdf(const FVec3 &, const FVec3 &, uint8_t) const noexcept override
        {
            return 0;
        }

        FSpectrum albedo() const noexcept override
        {
            return real(0.5) * (color_reflection_ + color_refraction_);
        }

        bool is_delta() const noexcept override
        {
            return true;
        }

        bool has_diffuse_component() const noexcept override
        {
            return false;
        }
    };
}

class Glass : public Material
{
    RC<const Texture2D> color_reflection_map_;
    RC<const Texture2D> color_refraction_map_;
    RC<const Texture2D> ior_;

    RC<const BSSRDFSurface> bssrdf_;

public:

    Glass(
        RC<const Texture2D> color_reflection_map,
        RC<const Texture2D> color_refraction_map,
        RC<const Texture2D> ior,
        RC<const BSSRDFSurface> bssrdf)
    {
        color_reflection_map_ = color_reflection_map;
        color_refraction_map_ = color_refraction_map;
        ior_                  = ior;

        bssrdf_ = std::move(bssrdf);
    }

    ShadingPoint shade(const EntityIntersection &inct, Arena &arena) const override
    {
        ShadingPoint ret;

        const real     ior               = ior_->sample_real(inct);
        const FSpectrum color_reflection = color_reflection_map_->sample_spectrum(inct);
        const FSpectrum color_refraction = color_refraction_map_->sample_spectrum(inct);

        const DielectricFresnelPoint *fresnel_point =
            arena.create<DielectricFresnelPoint>(ior, real(1));

        ret.bsdf = arena.create_nodestruct<GlassBSDF>(
            inct.geometry_coord, inct.user_coord, fresnel_point,
            color_reflection, color_refraction);
        ret.shading_normal = inct.user_coord.z;
        ret.bssrdf = bssrdf_->create(inct, arena);

        return ret;
    }
};

RC<Material> create_glass(
    RC<const Texture2D> color_reflection_map,
    RC<const Texture2D> color_refraction_map,
    RC<const Texture2D> ior,
    RC<const BSSRDFSurface> bssrdf)
{
    return newRC<Glass>(
        color_reflection_map, color_refraction_map, ior, std::move(bssrdf));
}

AGZ_TRACER_END
