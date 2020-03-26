#include <optional>

#include <agz/tracer/core/bsdf.h>
#include <agz/tracer/core/bssrdf.h>
#include <agz/tracer/core/material.h>
#include <agz/tracer/core/texture2d.h>
#include <agz/utility/misc.h>

#include "./utility/fresnel_point.h"

AGZ_TRACER_BEGIN

namespace
{
    class GlassBSDF : public LocalBSDF
    {
        const DielectricFresnelPoint *fresnel_point_;
        Spectrum color_reflection_;
        Spectrum color_refraction_;

        static std::optional<Vec3> refr_dir(
            const Vec3 &nwo, const Vec3 &nor, real eta)
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

        GlassBSDF(const Coord &geometry_coord, const Coord &shading_coord,
                  const DielectricFresnelPoint *fresnel_point,
                  const Spectrum &color_reflection,
                  const Spectrum &color_refraction) noexcept
            : LocalBSDF(geometry_coord, shading_coord),
              fresnel_point_(fresnel_point),
              color_reflection_(color_reflection),
              color_refraction_(color_refraction)
        {

        }

        Spectrum eval(const Vec3&, const Vec3&, TransMode) const noexcept override
        {
            return Spectrum();
        }

        BSDFSampleResult sample(
            const Vec3 &out_dir, TransMode transport_mode,
            const Sample3 &sam) const noexcept override
        {
            const Vec3 nwo = shading_coord_.global_to_local(out_dir).normalize();
            const Vec3 nor = nwo.z > 0 ? Vec3(0, 0, 1) : Vec3(0, 0, -1);

            const Spectrum fr = fresnel_point_->eval(nwo.z);
            if(sam.u < fr.r)
            {
                BSDFSampleResult ret;
                const Vec3 local_in = Vec3(-nwo.x, -nwo.y, nwo.z);
                ret.dir      = shading_coord_.local_to_global(local_in);
                ret.f        = color_reflection_ * fr / std::abs(local_in.z);
                ret.pdf      = fr.r;
                ret.is_delta = true;

                ret.f *= local_angle::normal_corr_factor(
                    geometry_coord_, shading_coord_, ret.dir);
                if(has_inf(ret.f))
                    return BSDF_SAMPLE_RESULT_INVALID;

                return ret;
            }

            const real eta_i = nwo.z > 0 ? fresnel_point_->eta_o()
                                         : fresnel_point_->eta_i();
            const real eta_t = nwo.z > 0 ? fresnel_point_->eta_i()
                                         : fresnel_point_->eta_o();
            const real eta = eta_i / eta_t;

            auto opt_wi = refr_dir(nwo, nor, eta);
            if(!opt_wi)
                return BSDF_SAMPLE_RESULT_INVALID;
            const Vec3 nwi = opt_wi->normalize();

            const real corr_factor = transport_mode == TransMode::Radiance ?
                                     eta * eta : real(1);

            BSDFSampleResult ret;
            ret.dir      = shading_coord_.local_to_global(nwi);
            ret.f        = corr_factor * color_refraction_
                         * (1 - fr.r) / std::abs(nwi.z);
            ret.pdf      = 1 - fr.r;
            ret.is_delta = true;

            ret.f *= local_angle::normal_corr_factor(
                geometry_coord_, shading_coord_, ret.dir);
            if(has_inf(ret.f))
                return BSDF_SAMPLE_RESULT_INVALID;

            return ret;
        }

        real pdf(const Vec3&, const Vec3&) const noexcept override
        {
            return 0;
        }

        Spectrum albedo() const noexcept override
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

        const real     ior              = ior_->sample_real(inct.uv);
        const Spectrum color_reflection = color_reflection_map_->sample_spectrum(inct.uv);
        const Spectrum color_refraction = color_refraction_map_->sample_spectrum(inct.uv);

        const DielectricFresnelPoint *fresnel_point =
            arena.create<DielectricFresnelPoint>(ior, real(1));

        ret.bsdf = arena.create<GlassBSDF>(
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
