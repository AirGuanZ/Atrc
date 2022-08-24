#include <agz/tracer/core/bsdf.h>
#include <agz/tracer/core/bssrdf.h>
#include <agz/tracer/core/material.h>
#include <agz/tracer/core/texture2d.h>
#include <agz/tracer/utility/reflection.h>
#include <agz-utils/misc.h>

#include "./utility/microfacet.h"

AGZ_TRACER_BEGIN

namespace disney_impl
{

    using math::mix;
    using math::sqr;

    class DisneyBSDF : public LocalBSDF
    {
        FSpectrum C_;
        FSpectrum Ctint_;

        real metallic_;
        real roughness_;
        FSpectrum specular_scale_;
        real specular_tint_;
        real anisotropic_;
        real sheen_;
        real sheen_tint_;
        real clearcoat_;

        real transmission_;
        real IOR_; // inner IOR / outer IOR

        real transmission_roughness_;
        real trans_ax_, trans_ay_;

        real ax_, ay_;
        real clearcoat_roughness_;

        static constexpr real SS_TRANS_ROUGH = real(0.01);
        struct SampleWeights
        {
            real diffuse          = real(0.25);
            real specular         = real(0.25);
            real clearcoat        = real(0.25);
            real transmission     = real(0.25);
        } sample_w;

        static real one_minus_5(real x) noexcept
        {
            const real t = 1 - x;
            const real t2 = t * t;
            return t2 * t2 * t;
        }

        static real eta_to_R0(real eta) noexcept
        {
            return sqr(eta - 1) / sqr(eta + 1);
        }

        static FSpectrum schlick(const FSpectrum &R0, real cos_theta) noexcept
        {
            return R0 + (FSpectrum(1) - R0) * one_minus_5(cos_theta);
        }

        static real schlick(const real &R0, real cos_theta) noexcept
        {
            return R0 + (1 - R0) * one_minus_5(cos_theta);
        }

        static FSpectrum to_tint(const FSpectrum &base_color) noexcept
        {
            const real lum = base_color.lum();
            return lum > 0 ? base_color / lum : FSpectrum(1);
        }

        FSpectrum f_diffuse(
            real cos_theta_i, real cos_theta_o, real cos_theta_d) const noexcept
        {
            const FSpectrum f_lambert = C_ / PI_r;
            const real FL = one_minus_5(cos_theta_i);
            const real FV = one_minus_5(cos_theta_o);
            const real RR = 2 * roughness_ * cos_theta_d * cos_theta_d;
            const FSpectrum F_retro_refl =
                C_ / PI_r * RR * (FL + FV + FL * FV * (RR - 1));

            return f_lambert * (1 - real(0.5) * FL) * (1 - real(0.5) * FV)
                 + F_retro_refl;
        }

        FSpectrum f_sheen(real cos_theta_d) const noexcept
        {
            return 4 * sheen_ * mix(FSpectrum(1), Ctint_, sheen_tint_)
                              * one_minus_5(cos_theta_d);
        }

        FSpectrum f_clearcoat(
            real cos_theta_i, real cos_theta_o,
            real tan_theta_i, real tan_theta_o,
            real sin_theta_h, real cos_theta_h, real cos_theta_d) const noexcept
        {
            assert(cos_theta_i > 0 && cos_theta_o > 0);
            const real D = microfacet::gtr1(
                sin_theta_h, cos_theta_h, clearcoat_roughness_);
            const real F = schlick(real(0.04), cos_theta_d);
            const real G = microfacet::smith_gtr2(tan_theta_i, real(0.25))
                         * microfacet::smith_gtr2(tan_theta_o, real(0.25));
            return FSpectrum(
                clearcoat_ * D * F * G / std::abs(4 * cos_theta_i * cos_theta_o));
        }

        FSpectrum f_trans(
            const FVec3 &lwi, const FVec3 &lwo, TransMode mode) const noexcept
        {
            assert(lwi.z * lwo.z < 0);

            const real cos_theta_i = local_angle::cos_theta(lwi);
            const real cos_theta_o = local_angle::cos_theta(lwo);

            const real eta = cos_theta_o > 0 ? IOR_ : 1 / IOR_;
            FVec3 lwh = (lwo + eta * lwi).normalize();
            if(lwh.z < 0)
                lwh = -lwh;

            const real cos_theta_d = dot(lwo, lwh);
            const real F = refl_aux::dielectric_fresnel(IOR_, 1, cos_theta_d);

            const real phi_h       = local_angle::phi(lwh);
            const real sin_phi_h   = std::sin(phi_h);
            const real cos_phi_h   = std::cos(phi_h);
            const real cos_theta_h = local_angle::cos_theta(lwh);
            const real sin_theta_h = local_angle::cos_2_sin(cos_theta_h);
            const real D = microfacet::anisotropic_gtr2(
                sin_phi_h, cos_phi_h,
                sin_theta_h, cos_theta_h,
                trans_ax_, trans_ay_);

            const real phi_i       = local_angle::phi(lwi);
            const real phi_o       = local_angle::phi(lwo);
            const real sin_phi_i   = std::sin(phi_i), cos_phi_i = std::cos(phi_i);
            const real sin_phi_o   = std::sin(phi_o), cos_phi_o = std::cos(phi_o);
            const real tan_theta_i = local_angle::tan_theta(lwi);
            const real tan_theta_o = local_angle::tan_theta(lwo);
            const real G = microfacet::smith_anisotropic_gtr2(
                                cos_phi_i, sin_phi_i,
                                trans_ax_, trans_ay_, tan_theta_i)
                         * microfacet::smith_anisotropic_gtr2(
                                cos_phi_o, sin_phi_o,
                                trans_ax_, trans_ay_, tan_theta_o);

            const real sdem = cos_theta_d + eta * dot(lwi, lwh);
            const real corr_factor = mode == TransMode::Radiance ? 1 / eta : 1;

            const FSpectrum sqrtC = sqrt(C_);

            const real val = (1 - F) * D * G * eta * eta
                           * dot(lwi, lwh) * dot(lwo, lwh)
                           * corr_factor * corr_factor
                           / (cos_theta_i * cos_theta_o * sdem * sdem);

            const real trans_factor = cos_theta_o > 0 ? transmission_ : 1;
            return (1 - metallic_) * trans_factor * sqrtC * std::abs(val);
        }

        FSpectrum f_inner_refl(const FVec3 &lwi, const FVec3 &lwo) const noexcept
        {
            assert(lwi.z < 0 && lwo.z < 0);
            
            const FVec3 lwh = -(lwi + lwo).normalize();
            assert(lwh.z > 0);

            const real cos_theta_d = dot(lwo, lwh);
            const real F = refl_aux::dielectric_fresnel(IOR_, 1, cos_theta_d);
            
            const real phi_h       = local_angle::phi(lwh);
            const real sin_phi_h   = std::sin(phi_h);
            const real cos_phi_h   = std::cos(phi_h);
            const real cos_theta_h = local_angle::cos_theta(lwh);
            const real sin_theta_h = local_angle::cos_2_sin(cos_theta_h);
            const real D = microfacet::anisotropic_gtr2(
                sin_phi_h, cos_phi_h,
                sin_theta_h, cos_theta_h,
                trans_ax_, trans_ay_);

            const real phi_i       = local_angle::phi(lwi);
            const real phi_o       = local_angle::phi(lwo);
            const real sin_phi_i   = std::sin(phi_i);
            const real cos_phi_i   = std::cos(phi_i);
            const real sin_phi_o   = std::sin(phi_o);
            const real cos_phi_o   = std::cos(phi_o);
            const real tan_theta_i = local_angle::tan_theta(lwi);
            const real tan_theta_o = local_angle::tan_theta(lwo);
            const real G = microfacet::smith_anisotropic_gtr2(
                                cos_phi_i, sin_phi_i,
                                trans_ax_, trans_ay_, tan_theta_i)
                         * microfacet::smith_anisotropic_gtr2(
                                cos_phi_o, sin_phi_o,
                                trans_ax_, trans_ay_, tan_theta_o);

            return transmission_ * C_ * std::abs(F * D * G / (4 * lwi.z * lwo.z));
        }

        FSpectrum f_specular(const FVec3 &lwi, const FVec3 &lwo) const noexcept
        {
            assert(lwi.z > 0 && lwo.z > 0);

            const real cos_theta_i = local_angle::cos_theta(lwi);
            const real cos_theta_o = local_angle::cos_theta(lwo);

            const FVec3 lwh = (lwi + lwo).normalize();
            const real cos_theta_d = dot(lwi, lwh);

            const FSpectrum Cspec = mix(
                mix(FSpectrum(1), Ctint_, specular_tint_),
                C_, metallic_);
            const FSpectrum dielectric_fresnel = Cspec
                * refl_aux::dielectric_fresnel(IOR_, 1, cos_theta_d);
            const FSpectrum conductor_fresnel = schlick(Cspec, cos_theta_d);
            const FSpectrum F = mix(
                specular_scale_ * dielectric_fresnel, conductor_fresnel, metallic_);
            
            const real phi_h       = local_angle::phi(lwh);
            const real sin_phi_h   = std::sin(phi_h);
            const real cos_phi_h   = std::cos(phi_h);
            const real cos_theta_h = local_angle::cos_theta(lwh);
            const real sin_theta_h = local_angle::cos_2_sin(cos_theta_h);
            const real D = microfacet::anisotropic_gtr2(
                sin_phi_h, cos_phi_h, sin_theta_h, cos_theta_h, ax_, ay_);
            
            const real phi_i       = local_angle::phi(lwi);
            const real phi_o       = local_angle::phi(lwo);
            const real sin_phi_i   = std::sin(phi_i), cos_phi_i = std::cos(phi_i);
            const real sin_phi_o   = std::sin(phi_o), cos_phi_o = std::cos(phi_o);
            const real tan_theta_i = local_angle::tan_theta(lwi);
            const real tan_theta_o = local_angle::tan_theta(lwo);
            const real G = microfacet::smith_anisotropic_gtr2(
                                cos_phi_i, sin_phi_i, ax_, ay_, tan_theta_i)
                         * microfacet::smith_anisotropic_gtr2(
                                cos_phi_o, sin_phi_o, ax_, ay_, tan_theta_o);

            return F * D * G / std::abs(4 * cos_theta_i * cos_theta_o);
        }

        FVec3 sample_diffuse(const Sample2 &sam) const noexcept
        {
            return math::distribution::zweighted_on_hemisphere(
                            sam.u, sam.v).first;
        }

        FVec3 sample_specular(const FVec3 &lwo, const Sample2 &sam) const noexcept
        {
            const FVec3 lwh = microfacet::sample_anisotropic_gtr2_vnor(
                lwo, ax_, ay_, sam).normalize();
            if(lwh.z <= 0)
                return {};

            const FVec3 lwi = (2 * dot(lwo, lwh) * lwh - lwo).normalize();
            if(lwi.z <= 0)
                return {};

            return lwi;
        }

        FVec3 sample_clearcoat(const FVec3 &lwo, const Sample2 &sam) const noexcept
        {
            const FVec3 lwh = microfacet::sample_gtr1(clearcoat_roughness_, sam);
            if(lwh.z <= 0)
                return {};

            const FVec3 lwi = (2 * dot(lwo, lwh) * lwh - lwo).normalize();
            if(lwi.z <= 0)
                return {};

            return lwi;
        }

        FVec3 sample_transmission(
            const FVec3 &lwo, const Sample2 &sam) const noexcept
        {
            const FVec3 lwh = microfacet::sample_anisotropic_gtr2(
                trans_ax_, trans_ay_, sam);
            if(lwh.z <= 0)
                return {};

            if((lwo.z > 0) != (dot(lwh, lwo) > 0))
                return {};

            const real eta = lwo.z > 0 ? 1 / IOR_ : IOR_;
            const FVec3 owh = dot(lwh, lwo) > 0 ? lwh : -lwh;
            auto opt_lwi = refl_aux::refract(lwo, owh, eta);
            if(!opt_lwi)
                return {};

            const FVec3 lwi = opt_lwi->normalize();
            if(lwi.z * lwo.z > 0 || ((lwi.z > 0) != (dot(lwh, lwi) > 0)))
                return {};

            return lwi;
        }

        FVec3 sample_inner_refl(
            const FVec3 &lwo, const Sample2 &sam) const noexcept
        {
            assert(lwo.z < 0);
            
            const FVec3 lwh = microfacet::sample_anisotropic_gtr2(
                trans_ax_, trans_ay_, sam);
            if(lwh.z <= 0)
                return {};

            const FVec3 lwi = (2 * dot(lwo, lwh) * lwh - lwo);
            if(lwi.z > 0)
                return {};
            return lwi.normalize();
        }

        real pdf_diffuse(const FVec3 &lwi, const FVec3 &lwo) const noexcept
        {
            assert(lwi.z > 0 && lwo.z > 0);
            return math::distribution::zweighted_on_hemisphere_pdf(lwi.z);
        }

        std::pair<real, real> pdf_specular_clearcoat(
            const FVec3 &lwi, const FVec3 &lwo) const noexcept
        {
            assert(lwi.z > 0 && lwo.z > 0);

            const FVec3 lwh = (lwi + lwo).normalize();
            const real phi_h       = local_angle::phi(lwh);
            const real sin_phi_h   = std::sin(phi_h);
            const real cos_phi_h   = std::cos(phi_h);
            const real cos_theta_h = local_angle::cos_theta(lwh);
            const real sin_theta_h = local_angle::cos_2_sin(cos_theta_h);
            const real cos_theta_d = dot(lwi, lwh);

            const real cos_phi_o = std::cos(local_angle::phi(lwo));
            const real sin_phi_o = local_angle::cos_2_sin(cos_phi_o);

            const real tan_theta_o = local_angle::tan_theta(lwo);

            const real specular_D = microfacet::anisotropic_gtr2(
                sin_phi_h, cos_phi_h, sin_theta_h, cos_theta_h, ax_, ay_);

            const real pdf_specular =
                microfacet::smith_anisotropic_gtr2(
                    cos_phi_o, sin_phi_o, ax_, ay_, tan_theta_o)
                * specular_D / (4 * lwo.z);

            const real clearcoat_D = microfacet::gtr1(
                sin_theta_h, cos_theta_h, clearcoat_roughness_);
            const real pdf_clearcoat = cos_theta_h * clearcoat_D / (4 * cos_theta_d);

            return { pdf_specular, pdf_clearcoat };
        }

        real pdf_transmission(const FVec3 &lwi, const FVec3 &lwo) const noexcept
        {
            assert(lwi.z * lwo.z < 0);

            const real eta = lwo.z > 0 ? IOR_ : 1 / IOR_;
            FVec3 lwh = (lwo + eta * lwi).normalize();
            if(lwh.z < 0)
                lwh = -lwh;

            if(((lwo.z > 0) != (dot(lwh, lwo) > 0)) ||
               ((lwi.z > 0) != (dot(lwh, lwi) > 0)))
                return 0;

            const real sdem = dot(lwo, lwh) + eta * dot(lwi, lwh);
            const real dwh_to_dwi = eta * eta * dot(lwi, lwh) / (sdem * sdem);

            const real phi_h       = local_angle::phi(lwh);
            const real sin_phi_h   = std::sin(phi_h);
            const real cos_phi_h   = std::cos(phi_h);
            const real cos_theta_h = local_angle::cos_theta(lwh);
            const real sin_theta_h = local_angle::cos_2_sin(cos_theta_h);

            const real D = microfacet::anisotropic_gtr2(
                sin_phi_h, cos_phi_h,
                sin_theta_h, cos_theta_h, trans_ax_, trans_ay_);
            return std::abs(cos_theta_h * D * dwh_to_dwi);
        }

        real pdf_inner_refl(const FVec3 &lwi, const FVec3 &lwo) const noexcept
        {
            assert(lwi.z < 0 && lwo.z < 0);
            
            const FVec3 lwh = -(lwi + lwo).normalize();
            const real phi_h       = local_angle::phi(lwh);
            const real sin_phi_h   = std::sin(phi_h);
            const real cos_phi_h   = std::cos(phi_h);
            const real cos_theta_h = local_angle::cos_theta(lwh);
            const real sin_theta_h = local_angle::cos_2_sin(cos_theta_h);
            const real cos_theta_d = dot(lwi, lwh);
            
            const real D = microfacet::anisotropic_gtr2(
                sin_phi_h, cos_phi_h,
                sin_theta_h, cos_theta_h, trans_ax_, trans_ay_);
            return std::abs(cos_theta_h * D / (4 * cos_theta_d));
        }

    public:

        DisneyBSDF(const FCoord &geometry_coord, const FCoord &shading_coord,
                   const FSpectrum &base_color,
                   real metallic,
                   real roughness,
                   const FSpectrum &specular_scale,
                   real specular_tint,
                   real anisotropic,
                   real sheen,
                   real sheen_tint,
                   real clearcoat,
                   real clearcoat_gloss,
                   real transmission,
                   real transmission_roughness,
                   real IOR)
            : LocalBSDF(geometry_coord, shading_coord)
        {
            C_     = base_color;
            Ctint_ = to_tint(base_color);

            metallic_       = metallic;
            roughness_      = roughness;
            specular_scale_ = specular_scale;
            specular_tint_  = specular_tint;
            anisotropic_    = anisotropic;
            sheen_          = sheen;
            sheen_tint_     = sheen_tint;

            transmission_  = transmission;
            transmission_roughness_ = transmission_roughness;
            IOR_           = (std::max)(real(1.01), IOR);

            const real aspect = anisotropic > 0 ?
                std::sqrt(1 - real(0.9) * anisotropic) : real(1);
            ax_ = std::max(real(0.001), sqr(roughness) / aspect);
            ay_ = std::max(real(0.001), sqr(roughness) * aspect);

            trans_ax_ = (std::max)(real(0.001), sqr(transmission_roughness) / aspect);
            trans_ay_ = (std::max)(real(0.001), sqr(transmission_roughness) * aspect);

            clearcoat_ = clearcoat;
            clearcoat_roughness_ = mix(real(0.1), real(0), clearcoat_gloss);
            clearcoat_roughness_ *= clearcoat_roughness_;
            clearcoat_roughness_ = (std::max)(clearcoat_roughness_, real(0.0001));

            const real A = (math::clamp)(
                base_color.lum() * (1 - metallic_), real(0.3), real(0.7));
            const real B = 1 - A;
            
            sample_w.diffuse      = A * (1 - transmission_);
            sample_w.transmission = A * transmission_;
            sample_w.specular     = B * 2 / (2 + clearcoat_);
            sample_w.clearcoat    = B * clearcoat_ / (2 + clearcoat_);
        }

        FSpectrum eval(const FVec3 &wi, const FVec3 &wo, TransMode mode) const override
        {
            if(cause_black_fringes(wi, wo))
                return eval_black_fringes(wi, wo);

            const FVec3 lwi = shading_coord_.global_to_local(wi).normalize();
            const FVec3 lwo = shading_coord_.global_to_local(wo).normalize();

            if(std::abs(lwi.z) < EPS() || std::abs(lwo.z) < EPS())
                return {};

            // transmission
            
            if(lwi.z * lwo.z < 0)
            {
                if(!transmission_)
                    return {};

                const FSpectrum value = f_trans(lwi, lwo, mode);
                return value * local_angle::normal_corr_factor(
                    geometry_coord_, shading_coord_, wi);
            }

            // inner refl

            if(lwi.z < 0 && lwo.z < 0)
            {
                if(!transmission_)
                    return {};

                const FSpectrum value = f_inner_refl(lwi, lwo);
                return value * local_angle::normal_corr_factor(
                    geometry_coord_, shading_coord_, wi);
            }

            // reflection

            if(lwi.z <= 0 || lwo.z <= 0)
                return {};

            const real cos_theta_i = local_angle::cos_theta(lwi);
            const real cos_theta_o = local_angle::cos_theta(lwo);

            const FVec3 lwh = (lwi + lwo).normalize();
            const real cos_theta_d = dot(lwi, lwh);

            FSpectrum diffuse, sheen;
            if(metallic_ < 1)
            {
                diffuse = f_diffuse(cos_theta_i, cos_theta_o, cos_theta_d);
                if(sheen_ > 0)
                    sheen = f_sheen(cos_theta_d);
            }

            FSpectrum specular = f_specular(lwi, lwo);

            FSpectrum clearcoat;
            if(clearcoat_ > 0)
            {
                const real tan_theta_i = local_angle::tan_theta(lwi);
                const real tan_theta_o = local_angle::tan_theta(lwo);
                const real cos_theta_h = local_angle::cos_theta(lwh);
                const real sin_theta_h = local_angle::cos_2_sin(cos_theta_h);

                clearcoat = f_clearcoat(
                    cos_theta_i, cos_theta_o, tan_theta_i, tan_theta_o,
                    sin_theta_h, cos_theta_h, cos_theta_d);
            }

            const FSpectrum value = (1 - metallic_) * (1 - transmission_)
                                 * (diffuse + sheen) + specular + clearcoat;

            const real normal_corr_factor = local_angle::normal_corr_factor(
                geometry_coord_, shading_coord_, wi);

            return value * normal_corr_factor;
        }

        BSDFSampleResult sample(const FVec3 &wo, TransMode mode, const Sample3 &sam) const override
        {
            if(cause_black_fringes(wo))
                return sample_black_fringes(wo, mode, sam);

            const FVec3 lwo = shading_coord_.global_to_local(wo).normalize();
            if(std::abs(lwo.z) < EPS())
                return BSDF_SAMPLE_RESULT_INVALID;

            // transmission and inner refl

            if(lwo.z < 0)
            {
                if(!transmission_)
                    return BSDF_SAMPLE_RESULT_INVALID;

                FVec3 lwi;
                real macro_F = refl_aux::dielectric_fresnel(IOR_, 1, lwo.z);
                macro_F = math::clamp(macro_F, real(0.1), real(0.9));
                if(sam.u >= macro_F)
                    lwi = sample_transmission(lwo, { sam.v, sam.w });
                else
                    lwi = sample_inner_refl(lwo, { sam.v, sam.w });

                if(!lwi)
                    return BSDF_SAMPLE_RESULT_INVALID;

                const FVec3     wi  = shading_coord_.local_to_global(lwi);
                const FSpectrum f   = eval(wi, wo, mode);
                const real      pdf = this->pdf(wi, wo);

                return BSDFSampleResult(wi, f, pdf, false);
            }

            // reflection + transmission

            real sam_selector = sam.u;
            const Sample2 new_sam{ sam.v, sam.w };

            FVec3 lwi;

            if(sam_selector < sample_w.diffuse)
                lwi = sample_diffuse(new_sam);
            else
                sam_selector -= sample_w.diffuse;

            if(sam_selector < sample_w.transmission)
                lwi = sample_transmission(lwo, new_sam);
            else if(sam_selector -= sample_w.transmission;
                sam_selector < sample_w.specular)
                lwi = sample_specular(lwo, new_sam);
            else
                lwi = sample_clearcoat(lwo, new_sam);

            if(!lwi)
                return BSDF_SAMPLE_RESULT_INVALID;
            
            const FVec3 wi    = shading_coord_.local_to_global(lwi);
            const FSpectrum f = eval(wi, wo, mode);
            const real pdf    = this->pdf(wi, wo);

            return BSDFSampleResult(wi, f, pdf, false);
        }

        BSDFBidirSampleResult sample_bidir(const FVec3 &wo, TransMode mode, const Sample3 &sam) const override
        {
            auto t = sample(wo, mode, sam);
            if(t.invalid())
                return BSDF_BIDIR_SAMPLE_RESULT_INVALID;
            BSDFBidirSampleResult ret(UNINIT);
            ret.dir = t.dir;
            ret.f = t.f;
            ret.pdf = t.pdf;
            ret.pdf_rev = pdf(wo, ret.dir);
            return ret;
        }

        real pdf(const FVec3 &wi, const FVec3 &wo) const override
        {
            if(cause_black_fringes(wi, wo))
                return pdf_for_black_fringes(wi, wo);

            const FVec3 lwi = shading_coord_.global_to_local(wi).normalize();
            const FVec3 lwo = shading_coord_.global_to_local(wo).normalize();
            if(std::abs(lwi.z) < EPS() || std::abs(lwo.z) < EPS())
                return 0;

            // transmission and inner refl

            if(lwo.z < 0)
            {
                if(!transmission_)
                    return 0;

                real macro_F = refl_aux::dielectric_fresnel(IOR_, 1, lwo.z);
                macro_F = math::clamp(macro_F, real(0.1), real(0.9));
                if(lwi.z > 0)
                    return (1 - macro_F) * pdf_transmission(lwi, lwo);
                if(!macro_F)
                    return 0;
                return macro_F * pdf_inner_refl(lwi, lwo);
            }

            // transmission and refl

            if(lwi.z < 0)
            {
                return sample_w.transmission * pdf_transmission(lwi, lwo);
            }

            const real diffuse = pdf_diffuse(lwi, lwo);

            auto [specular, clearcoat] = pdf_specular_clearcoat(lwi, lwo);

            return sample_w.diffuse   * diffuse
                 + sample_w.specular  * specular
                 + sample_w.clearcoat * clearcoat;
        }

        FSpectrum albedo() const override
        {
            return C_;
        }

        bool is_delta() const override
        {
            return false;
        }

        bool has_diffuse_component() const override
        {
            return metallic_ < 1 && transmission_ < 1;
        }
    };

} // namespace disney_impl

class Disney : public Material
{
    RC<const Texture2D> base_color_;
    RC<const Texture2D> metallic_;
    RC<const Texture2D> roughness_;
    RC<const Texture2D> specular_scale_;
    RC<const Texture2D> specular_tint_;
    RC<const Texture2D> anisotropic_;
    RC<const Texture2D> sheen_;
    RC<const Texture2D> sheen_tint_;
    RC<const Texture2D> clearcoat_;
    RC<const Texture2D> clearcoat_gloss_;
    RC<const Texture2D> transmission_;
    RC<const Texture2D> transmission_roughness_;
    RC<const Texture2D> IOR_;

    Box<const NormalMapper> normal_mapper_;
    RC<const BSSRDFSurface> bssrdf_;

public:

    Disney(
        RC<const Texture2D> base_color,
        RC<const Texture2D> metallic,
        RC<const Texture2D> roughness,
        RC<const Texture2D> transmission,
        RC<const Texture2D> transmission_roughness,
        RC<const Texture2D> ior,
        RC<const Texture2D> specular_scale,
        RC<const Texture2D> specular_tint,
        RC<const Texture2D> anisotropic,
        RC<const Texture2D> sheen,
        RC<const Texture2D> sheen_tint,
        RC<const Texture2D> clearcoat,
        RC<const Texture2D> clearcoat_gloss,
        Box<const NormalMapper> normal_mapper,
        RC<const BSSRDFSurface> bssrdf)
    {
        base_color_             = base_color;
        metallic_               = metallic;
        roughness_              = roughness;
        transmission_           = transmission;
        transmission_roughness_ = transmission_roughness;
        IOR_                    = ior;
        specular_scale_         = specular_scale;
        specular_tint_          = specular_tint;
        anisotropic_            = anisotropic;
        sheen_                  = sheen;
        sheen_tint_             = sheen_tint;
        clearcoat_              = clearcoat;
        clearcoat_gloss_        = clearcoat_gloss;

        normal_mapper_ = std::move(normal_mapper);
        bssrdf_ = std::move(bssrdf);
    }

    ShadingPoint shade(const EntityIntersection &inct, Arena &arena) const override
    {
        const Vec2 uv = inct.uv;
        const FSpectrum base_color             = base_color_      ->sample_spectrum(uv);
        const real     metallic               = metallic_        ->sample_real(uv);
        const real     roughness              = roughness_       ->sample_real(uv);
        const real     transmission           = transmission_    ->sample_real(uv);
        const real     transmission_roughness = transmission_roughness_->sample_real(uv);
        const real     ior                    = IOR_             ->sample_real(uv);
        const FSpectrum specular_scale         = specular_scale_  ->sample_spectrum(uv);
        const real     specular_tint          = specular_tint_   ->sample_real(uv);
        const real     anisotropic            = anisotropic_     ->sample_real(uv);
        const real     sheen                  = sheen_           ->sample_real(uv);
        const real     sheen_tint             = sheen_tint_      ->sample_real(uv);
        const real     clearcoat              = clearcoat_       ->sample_real(uv);
        const real     clearcoat_gloss        = clearcoat_gloss_ ->sample_real(uv);

        const FCoord shading_coord = normal_mapper_->reorient(uv, inct.user_coord);
        const BSDF *bsdf = arena.create_nodestruct<disney_impl::DisneyBSDF>(
            inct.geometry_coord, shading_coord,
            base_color,
            metallic,
            roughness,
            specular_scale,
            specular_tint,
            anisotropic,
            sheen,
            sheen_tint,
            clearcoat,
            clearcoat_gloss,
            transmission,
            transmission_roughness,
            ior);

        const BSSRDF *bssrdf = bssrdf_->create(inct, arena);

        ShadingPoint shd = { bsdf, shading_coord.z, bssrdf };
        return shd;
    }
};

RC<Material> create_disney(
    RC<const Texture2D> base_color,
    RC<const Texture2D> metallic,
    RC<const Texture2D> roughness,
    RC<const Texture2D> transmission,
    RC<const Texture2D> transmission_roughness,
    RC<const Texture2D> ior,
    RC<const Texture2D> specular_scale,
    RC<const Texture2D> specular_tint,
    RC<const Texture2D> anisotropic,
    RC<const Texture2D> sheen,
    RC<const Texture2D> sheen_tint,
    RC<const Texture2D> clearcoat,
    RC<const Texture2D> clearcoat_gloss,
    Box<const NormalMapper> normal_mapper,
    RC<const BSSRDFSurface> bssrdf)
{
    return newRC<Disney>(base_color,
        metallic,
        roughness,
        transmission,
        transmission_roughness,
        ior,
        specular_scale,
        specular_tint,
        anisotropic,
        sheen,
        sheen_tint,
        clearcoat,
        clearcoat_gloss,
        std::move(normal_mapper),
        std::move(bssrdf));
}

AGZ_TRACER_END
