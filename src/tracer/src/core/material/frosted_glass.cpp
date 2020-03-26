#include <optional>

#include <agz/tracer/core/bsdf.h>
#include <agz/tracer/core/bssrdf.h>
#include <agz/tracer/core/material.h>
#include <agz/tracer/core/texture2d.h>
#include <agz/utility/misc.h>

#include "./utility/fresnel_point.h"
#include "./utility/microfacet.h"

AGZ_TRACER_BEGIN

// See https://www.cs.cornell.edu/~srm/publications/EGSR07-btdf.html

namespace
{
    
    class FrostedGlassBSDF : public LocalBSDF
    {
        Spectrum color_;
        real alpha_;
        const DielectricFresnelPoint *fresnel_;

        bool cause_normal_conflict(const Vec3 &h, const Vec3 &w) const
        {
            const bool macro_posi = w.z > 0;
            const bool micro_posi = dot(h, w) > 0;
            return macro_posi != micro_posi;
        }

        Spectrum eval_reflection(const Vec3 &lwi, const Vec3 &lwo) const noexcept
        {
            Vec3 h = lwi + lwo;
            if(h.length_square() < EPS)
                return {};
            h = h.normalize();
            const Vec3 posh = h.z > 0 ? h : -h;

            const Spectrum Fr = fresnel_->eval(dot(posh, lwo));
            const real D = microfacet::gtr2(local_angle::cos_theta(posh), alpha_);

            const real tan_theta_i = local_angle::tan_theta(lwi);
            const real tan_theta_o = local_angle::tan_theta(lwo);
            const real G = std::abs(microfacet::smith_gtr2(tan_theta_i, alpha_)
                         * microfacet::smith_gtr2(tan_theta_o, alpha_));

            const Spectrum ret = color_ * Fr * std::abs(D * G / (4 * lwi.z * lwo.z));
            return ret.is_finite() ? ret : Spectrum();
        }

        Spectrum eval_refraction(
            const Vec3 &lwi, const Vec3 &lwo, TransMode mode) const noexcept
        {
            assert(lwi.z * lwo.z < 0);

            real eta_i = fresnel_->eta_o(), eta_t = fresnel_->eta_i();
            const bool is_entering = lwo.z > 0;
            if(!is_entering)
                std::swap(eta_i, eta_t);

            Vec3 h = -(eta_i * lwo + eta_t * lwi);
            if(h.length_square() < EPS)
                return {};
            h = h.normalize();
            const Vec3 posh = h.z > 0 ? h : -h;

            const real Fr = fresnel_->eval(dot(lwo, posh)).r;
            const real D = microfacet::gtr2(local_angle::cos_theta(posh), alpha_);

            const real tan_theta_i = local_angle::tan_theta(lwi);
            const real tan_theta_o = local_angle::tan_theta(lwo);
            const real G = std::abs(microfacet::smith_gtr2(tan_theta_i, alpha_)
                         * microfacet::smith_gtr2(tan_theta_o, alpha_));

            const real ho = dot(lwo, posh), hi = dot(lwi, posh);
            const real sdem = eta_i * ho + eta_t * hi;
            real val = (1 - Fr) * D * G * eta_t * eta_t * ho * hi
                     / (sdem * sdem * lwi.z * lwo.z);

            if(mode == TransMode::Importance)
                val *= eta_i * eta_i / (eta_t * eta_t);

            if(!std::isfinite(val))
                return {};

            return color_ * Spectrum(std::abs(val));
        }

        std::optional<Vec3> refract(
            const Vec3 &wi, const Vec3 &nor, real eta) const
        {
            const real cos_theta_i = dot(wi, nor);
            if(cos_theta_i < 0)
                eta = 1 / eta;
            const real sqr_cos_theta_t = 1 - (1 - cos_theta_i * cos_theta_i) * eta * eta;
            if(sqr_cos_theta_t <= 0.0f)
                return std::nullopt;
            const real sign = cos_theta_i >= 0 ? real(1) : real(-1);
            return nor * (-cos_theta_i * eta + sign * std::sqrt(sqr_cos_theta_t)) + wi * eta;
        }

    public:

        FrostedGlassBSDF(
            const Coord &geometry_coord, const Coord &shading_coord,
            const Spectrum &color, real alpha,
            const DielectricFresnelPoint *fresnel) noexcept
            : LocalBSDF(geometry_coord, shading_coord),
              color_(color), alpha_(alpha), fresnel_(fresnel)
        {
            
        }

        Spectrum eval(
            const Vec3 &wi, const Vec3 &wo, TransMode mode) const noexcept override
        {
            // black fringes

            if(cause_black_fringes(wi) || cause_black_fringes(wo))
                return {};

            // eval

            const Vec3 lwi = shading_coord_.global_to_local(wi).normalize();
            const Vec3 lwo = shading_coord_.global_to_local(wo).normalize();
            if(!lwi.z || !lwo.z)
                return {};

            const bool is_reflection = lwi.z * lwo.z > 0;
            Spectrum ret = is_reflection ?
                           eval_reflection(lwi, lwo) :
                           eval_refraction(lwi, lwo, mode);
            ret *= local_angle::normal_corr_factor(
                geometry_coord_, shading_coord_, wi);
            return ret;
        }

        BSDFSampleResult sample(
            const Vec3 &wo, TransMode mode, const Sample3 &sam) const noexcept override
        {
            if(cause_black_fringes(wo))
                return BSDF_SAMPLE_RESULT_INVALID;

            const Vec3 lwo = shading_coord_.global_to_local(wo).normalize();
            const Vec3 h = microfacet::sample_gtr2(alpha_, { sam.u, sam.v });
            const real D = microfacet::gtr2(local_angle::cos_theta(h), alpha_);
            const real pdf_h = local_angle::cos_theta(h) * D;

            if(!D || pdf_h < EPS || cause_normal_conflict(h, lwo))
                return BSDF_SAMPLE_RESULT_INVALID;

            const real Fr = fresnel_->eval(dot(lwo, h)).r;
            if(sam.w <= Fr) // reflection
            {
                const Vec3 lwi = (2 * dot(lwo, h) * h - lwo).normalize();
                if(cause_normal_conflict(h, lwi) || lwi.z * lwo.z < 0)
                    return BSDF_SAMPLE_RESULT_INVALID;

                const real tan_theta_i = local_angle::tan_theta(lwi);
                const real tan_theta_o = local_angle::tan_theta(lwo);
                const real G = std::abs(microfacet::smith_gtr2(tan_theta_i, alpha_)
                             * microfacet::smith_gtr2(tan_theta_o, alpha_));

                BSDFSampleResult ret;
                ret.f        = color_ * Fr * D * G / std::abs(4 * lwi.z * lwo.z);
                ret.pdf      = pdf_h * Fr / std::abs(4 * dot(lwo, h));
                ret.is_delta = false;
                ret.dir      = shading_coord_.local_to_global(lwi);

                if(cause_black_fringes(ret.dir) || !ret.f.is_finite() || ret.pdf < EPS)
                    return BSDF_SAMPLE_RESULT_INVALID;

                ret.f *= local_angle::normal_corr_factor(
                    geometry_coord_, shading_coord_, ret.dir);
                return ret;
            }

            // refraction

            const auto opt_wi = refract(-lwo, h, fresnel_->eta_i() / fresnel_->eta_o());
            if(!opt_wi || opt_wi->z * lwo.z >= 0)
                return BSDF_SAMPLE_RESULT_INVALID;
            const Vec3 lwi = opt_wi->normalize();
            if(cause_normal_conflict(h, lwi))
                return BSDF_SAMPLE_RESULT_INVALID;

            real eta_t = fresnel_->eta_i(), eta_i = fresnel_->eta_o();
            const bool is_entering = lwo.z > 0;
            if(!is_entering)
                std::swap(eta_t, eta_i);

            const real ho = dot(h, lwo), hi = dot(h, lwi);
            const real sdem = eta_i * ho + eta_t * hi;
            if(std::abs(sdem) < EPS)
                return BSDF_SAMPLE_RESULT_INVALID;

            const real tan_theta_i = local_angle::tan_theta(lwi);
            const real tan_theta_o = local_angle::tan_theta(lwo);
            const real G = std::abs(microfacet::smith_gtr2(tan_theta_i, alpha_)
                         * microfacet::smith_gtr2(tan_theta_o, alpha_));

            const real dhdwi = eta_t * eta_t * hi / (sdem * sdem);
            real val = (1 - Fr) * D * G * eta_t * eta_t * ho * hi
                     / (sdem * sdem * lwo.z * lwi.z);

            if(mode == TransMode::Importance)
                val *= eta_i * eta_i / (eta_t * eta_t);

            BSDFSampleResult ret;
            ret.dir      = shading_coord_.local_to_global(lwi);
            ret.f        = color_ * std::abs(val);
            ret.pdf      = std::abs(pdf_h * dhdwi * (1 - Fr));
            ret.is_delta = false;

            if(cause_black_fringes(ret.dir) || !ret.f.is_finite() || ret.pdf < EPS)
                return BSDF_SAMPLE_RESULT_INVALID;

            ret.f *= local_angle::normal_corr_factor(
                geometry_coord_, shading_coord_, ret.dir);
            return ret;
        }

        real pdf(const Vec3 &wi, const Vec3 &wo) const noexcept override
        {
            if(cause_black_fringes(wi) || cause_black_fringes(wo))
                return 0;

            const Vec3 lwi = shading_coord_.global_to_local(wi).normalize();
            const Vec3 lwo = shading_coord_.global_to_local(wo).normalize();
            if(!lwi.z || !lwo.z)
                return {};

            const bool is_reflection = lwi.z * lwo.z > 0;
            const bool is_entering = lwo.z > 0;

            Vec3 h; real dhdwi;
            if(is_reflection)
            {
                h = (lwo + lwi).normalize();
                dhdwi = 1 / (4 * dot(lwo, h));
            }
            else
            {
                real eta_i = fresnel_->eta_o(), eta_t = fresnel_->eta_i();
                if(!is_entering)
                    std::swap(eta_i, eta_t);
                h = -(eta_i * lwo + eta_t * lwi).normalize();

                const real ho = dot(lwo, h), hi = dot(lwi, h);
                const real sdem = eta_i * ho + eta_t * hi;
                dhdwi = (eta_t * eta_t * hi) / (sdem * sdem);
            }

            if(h.z < 0)
                h = -h;

            const real D = microfacet::gtr2(local_angle::cos_theta(h), alpha_);
            const real h_pdf = D * local_angle::cos_theta(h);
            const real Fr = fresnel_->eval(dot(lwo, h)).r;
            const real Fr_pdf = is_reflection ? Fr : 1 - Fr;
            const real ret = std::abs(h_pdf * Fr_pdf * dhdwi);
            return EPS < ret && std::isfinite(ret) ? ret : 0;
        }

        Spectrum albedo() const noexcept override
        {
            return color_;
        }

        bool is_delta() const noexcept override
        {
            return false;
        }

        bool has_diffuse_component() const noexcept override
        {
            return false;
        }
    };

} // namespace anonymous

class FrostedGlass : public Material
{
    RC<const Texture2D> ior_;
    RC<const Texture2D> color_map_;
    RC<const Texture2D> roughness_;

    RC<const BSSRDFSurface> bssrdf_;

public:

    FrostedGlass(
        RC<const Texture2D> color_map,
        RC<const Texture2D> roughness,
        RC<const Texture2D> ior,
        RC<const BSSRDFSurface> bssrdf)
    {
        color_map_ = std::move(color_map);
        ior_       = std::move(ior);
        roughness_ = std::move(roughness);

        bssrdf_ = std::move(bssrdf);
    }

    ShadingPoint shade(const EntityIntersection &inct, Arena &arena) const override
    {
        const Spectrum color = color_map_->sample_spectrum(inct.uv);
        const real roughness = math::clamp<real>(
            roughness_->sample_real(inct.uv), real(0.01), 1);
        const real ior = ior_->sample_real(inct.uv);

        const DielectricFresnelPoint *fresnel =
            arena.create<DielectricFresnelPoint>(ior, real(1));

        const BSDF *bsdf = arena.create<FrostedGlassBSDF>(
            inct.geometry_coord, inct.user_coord, color, roughness, fresnel);

        const BSSRDF *bssrdf = bssrdf_->create(inct, arena);

        return { bsdf, inct.user_coord.z, bssrdf };
    }
};

RC<Material> create_frosted_glass(
    RC<const Texture2D> color_map,
    RC<const Texture2D> roughness,
    RC<const Texture2D> ior,
    RC<const BSSRDFSurface> bssrdf)
{
    return newRC<FrostedGlass>(color_map, roughness, ior, std::move(bssrdf));
}

AGZ_TRACER_END
