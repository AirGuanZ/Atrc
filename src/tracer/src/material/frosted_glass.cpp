#include <optional>

#include <agz/tracer/core/bsdf.h>
#include <agz/tracer/core/fresnel.h>
#include <agz/tracer/core/material.h>
#include <agz/tracer/core/texture.h>

#include "./microfacet.h"

AGZ_TRACER_BEGIN

// See https://www.cs.cornell.edu/~srm/publications/EGSR07-btdf.html

namespace
{
    
    class FrostedGlassBSDF : public LocalBSDF
    {
        Spectrum color_;
        real alpha_;
        const FresnelPoint *fresnel_;

        // w位于micro coord和macro coord的z=0平面的夹缝中间的情况
        bool cause_normal_conflict(const Vec3 &h, const Vec3 &w) const
        {
            bool macro_posi = w.z > 0;
            bool micro_posi = dot(h, w) > 0;
            return macro_posi != micro_posi;
        }

        // 已知lwi.z * lwo.z > 0，求brdf
        Spectrum eval_reflection(const Vec3 &lwi, const Vec3 &lwo) const noexcept
        {
            Vec3 h = lwi + lwo;
            if(h.length_square() < EPS)
                return {};
            h = h.normalize();
            Vec3 posh = h.z > 0 ? h : -h;

            Spectrum Fr = fresnel_->eval(dot(posh, lwo));
            real D = microfacet::gtr2(local_angle::cos_theta(posh), alpha_);

            real tan_theta_i = local_angle::tan_theta(lwi);
            real tan_theta_o = local_angle::tan_theta(lwo);
            real G = std::abs(microfacet::smith_gtr2(tan_theta_i, alpha_) * microfacet::smith_gtr2(tan_theta_o, alpha_));

            Spectrum ret = color_ * Fr * std::abs(D * G / (4 * lwi.z * lwo.z));
            return ret.is_finite() ? ret : Spectrum();
        }

        Spectrum eval_refraction(const Vec3 &lwi, const Vec3 &lwo, TransportMode mode) const noexcept
        {
            assert(lwi.z * lwo.z < 0);

            real eta_i = fresnel_->eta_o(), eta_t = fresnel_->eta_i();
            bool is_entering = lwo.z > 0;
            if(!is_entering)
                std::swap(eta_i, eta_t);

            Vec3 h = -(eta_i * lwo + eta_t * lwi);
            if(h.length_square() < EPS)
                return {};
            h = h.normalize();
            Vec3 posh = h.z > 0 ? h : -h;

            real Fr = fresnel_->eval(dot(lwo, posh)).r;
            real D = microfacet::gtr2(local_angle::cos_theta(posh), alpha_);

            real tan_theta_i = local_angle::tan_theta(lwi);
            real tan_theta_o = local_angle::tan_theta(lwo);
            real G = std::abs(microfacet::smith_gtr2(tan_theta_i, alpha_) * microfacet::smith_gtr2(tan_theta_o, alpha_));

            real ho = dot(lwo, posh), hi = dot(lwi, posh);
            real sdem = eta_i * ho + eta_t * hi;
            real val = (1 - Fr) * D * G * eta_t * eta_t * ho * hi / (sdem * sdem * lwi.z * lwo.z);

            if(mode == TM_Importance)
                val *= eta_i * eta_i / (eta_t * eta_t);

            if(!isfinite(val))
                return {};

            return color_ * Spectrum(std::abs(val));
        }

        std::optional<Vec3> refract(const Vec3 &wi, const Vec3 &nor, real eta) const
        {
            real cos_theta_i = dot(wi, nor);
            if(cos_theta_i < 0)
                eta = 1 / eta;
            real sqr_cos_theta_t = 1 - (1 - cos_theta_i * cos_theta_i) * eta * eta;
            if(sqr_cos_theta_t <= 0.0f)
                return std::nullopt;
            float sign = cos_theta_i >= 0.0f ? 1.0f : -1.0f;
            return nor * (-cos_theta_i * eta + sign * std::sqrt(sqr_cos_theta_t)) + wi * eta;
        }

    public:

        FrostedGlassBSDF(
            const Coord &geometry_coord, const Coord &shading_coord,
            const Spectrum &color, real alpha, const FresnelPoint *fresnel) noexcept
            : LocalBSDF(geometry_coord, shading_coord),
              color_(color), alpha_(alpha), fresnel_(fresnel)
        {
            
        }

        Spectrum eval(const Vec3 &wi, const Vec3 &wo, TransportMode mode) const noexcept override
        {
            // black fringes

            if(cause_black_fringes(wi) || cause_black_fringes(wo))
                return {};

            // eval

            Vec3 lwi = shading_coord_.global_to_local(wi).normalize();
            Vec3 lwo = shading_coord_.global_to_local(wo).normalize();
            if(!lwi.z || !lwo.z)
                return {};

            bool is_reflection = lwi.z * lwo.z > 0;
            auto ret = is_reflection ? eval_reflection(lwi, lwo) : eval_refraction(lwi, lwo, mode);
            ret *= local_angle::normal_corr_factor(geometry_coord_, shading_coord_, wi);
            return ret;
        }

        BSDFSampleResult sample(const Vec3 &wo, TransportMode mode, const Sample3 &sam) const noexcept override
        {
            if(cause_black_fringes(wo))
                return BSDF_SAMPLE_RESULT_INVALID;

            Vec3 lwo = shading_coord_.global_to_local(wo).normalize();
            Vec3 h = microfacet::sample_gtr2(alpha_, { sam.u, sam.v });
            real D = microfacet::gtr2(local_angle::cos_theta(h), alpha_);
            real pdf_h = local_angle::cos_theta(h) * D;

            if(!D || pdf_h < EPS || cause_normal_conflict(h, lwo))
                return BSDF_SAMPLE_RESULT_INVALID;

            real Fr = fresnel_->eval(dot(lwo, h)).r;
            if(sam.w <= Fr) // reflection
            {
                Vec3 lwi = (2 * dot(lwo, h) * h - lwo).normalize();
                if(cause_normal_conflict(h, lwi) || lwi.z * lwo.z < 0)
                    return BSDF_SAMPLE_RESULT_INVALID;

                real tan_theta_i = local_angle::tan_theta(lwi);
                real tan_theta_o = local_angle::tan_theta(lwo);
                real G = std::abs(microfacet::smith_gtr2(tan_theta_i, alpha_) * microfacet::smith_gtr2(tan_theta_o, alpha_));

                BSDFSampleResult ret;
                ret.f        = color_ * Fr * D * G / std::abs(4 * lwi.z * lwo.z);
                ret.pdf      = pdf_h * Fr / std::abs(4 * dot(lwo, h));
                ret.is_delta = false;
                ret.dir      = shading_coord_.local_to_global(lwi);
                ret.mode     = mode;

                if(cause_black_fringes(ret.dir) || !ret.f.is_finite() || ret.pdf < EPS)
                    return BSDF_SAMPLE_RESULT_INVALID;

                ret.f *= local_angle::normal_corr_factor(geometry_coord_, shading_coord_, ret.dir);
                return ret;
            }

            // refraction

            auto opt_wi = refract(-lwo, h, fresnel_->eta_i() / fresnel_->eta_o());
            if(!opt_wi || opt_wi->z * lwo.z >= 0)
                return BSDF_SAMPLE_RESULT_INVALID;
            Vec3 lwi = opt_wi->normalize();
            if(cause_normal_conflict(h, lwi))
                return BSDF_SAMPLE_RESULT_INVALID;

            real eta_t = fresnel_->eta_i(), eta_i = fresnel_->eta_o();
            bool is_entering = lwo.z > 0;
            if(!is_entering)
                std::swap(eta_t, eta_i);

            real ho = dot(h, lwo), hi = dot(h, lwi);
            real sdem = eta_i * ho + eta_t * hi;
            if(std::abs(sdem) < EPS)
                return BSDF_SAMPLE_RESULT_INVALID;

            real tan_theta_i = local_angle::tan_theta(lwi);
            real tan_theta_o = local_angle::tan_theta(lwo);
            real G = std::abs(microfacet::smith_gtr2(tan_theta_i, alpha_) * microfacet::smith_gtr2(tan_theta_o, alpha_));

            real dhdwi = eta_t * eta_t * hi / (sdem * sdem);
            real val = (1 - Fr) * D * G * eta_t * eta_t * ho * hi / (sdem * sdem * lwo.z * lwi.z);

            if(mode == TM_Importance)
                val *= eta_i * eta_i / (eta_t * eta_t);

            BSDFSampleResult ret;
            ret.dir      = shading_coord_.local_to_global(lwi);
            ret.f        = color_ * std::abs(val);
            ret.pdf      = std::abs(pdf_h * dhdwi * (1 - Fr));
            ret.is_delta = false;
            ret.mode     = mode;

            if(cause_black_fringes(ret.dir) || !ret.f.is_finite() || ret.pdf < EPS)
                return BSDF_SAMPLE_RESULT_INVALID;

            ret.f *= local_angle::normal_corr_factor(geometry_coord_, shading_coord_, ret.dir);
            return ret;
        }

        real pdf(const Vec3 &wi, const Vec3 &wo, TransportMode mode) const noexcept override
        {
            if(cause_black_fringes(wi) || cause_black_fringes(wo))
                return 0;

            Vec3 lwi = shading_coord_.global_to_local(wi).normalize();
            Vec3 lwo = shading_coord_.global_to_local(wo).normalize();
            if(!lwi.z || !lwo.z)
                return {};

            bool is_reflection = lwi.z * lwo.z > 0;
            bool is_entering = lwo.z > 0;

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

                real ho = dot(lwo, h), hi = dot(lwi, h);
                real sdem = eta_i * ho + eta_t * hi;
                dhdwi = (eta_t * eta_t * hi) / (sdem * sdem);
            }

            if(h.z < 0)
                h = -h;
            real D = microfacet::gtr2(local_angle::cos_theta(h), alpha_);
            real h_pdf = D * local_angle::cos_theta(h);
            real Fr = fresnel_->eval(dot(lwo, h)).r;
            real Fr_pdf = is_reflection ? Fr : 1 - Fr;

            real ret = std::abs(h_pdf * Fr_pdf * dhdwi);
            return EPS < ret && isfinite(ret) ? ret : 0;
        }

        Spectrum albedo() const noexcept override
        {
            return color_;
        }

        bool is_delta() const noexcept override
        {
            return false;
        }

        bool is_black() const noexcept override
        {
            return !color_;
        }
    };

} // namespace anonymous

class FrostedGlass : public Material
{
    const Fresnel *fresnel_ = nullptr;
    const Texture *color_map_ = nullptr;
    const Texture *roughness_ = nullptr;

public:

    using Material::Material;

    static std::string description()
    {
        return R"___(
frosted_glass [Material]
    color_map [Texture] color map
    fresnel   [Fresnel] fresnel term
    roughness [Texture] roughness map
)___";
    }

    void initialize(const Config &params, obj::ObjectInitContext &init_ctx) override
    {
        AGZ_HIERARCHY_TRY

        color_map_ = TextureFactory.create(params.child_group("color_map"), init_ctx);
        fresnel_   = FresnelFactory.create(params.child_group("fresnel"),   init_ctx);
        roughness_ = TextureFactory.create(params.child_group("roughness"), init_ctx);

        AGZ_HIERARCHY_WRAP("in initializing frosted glass material")
    }

    ShadingPoint shade(const EntityIntersection &inct, Arena &arena) const override
    {
        auto color     = color_map_->sample_spectrum(inct.uv);
        auto fresnel   = fresnel_->get_point(inct.uv, arena);
        auto roughness = roughness_->sample_real(inct.uv);
        auto bsdf = arena.create<FrostedGlassBSDF>(
            inct.geometry_coord, inct.user_coord, color, roughness, fresnel);
        return { bsdf };
    }
};

AGZT_IMPLEMENTATION(Material, FrostedGlass, "frosted_glass")

AGZ_TRACER_END
