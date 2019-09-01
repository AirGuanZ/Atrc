#include <agz/tracer/core/bsdf.h>
#include <agz/tracer/core/material.h>
#include <agz/tracer/core/texture.h>
#include <agz/tracer/utility/reflection.h>

#include "./bssrdf/normalized_diffusion.h"
#include "./utility/microfacet.h"
#include "./utility/normal_mapper.h"

AGZ_TRACER_BEGIN

namespace disney_impl
{

    using math::mix;
    using math::sqr;

    class DisneyBSDF : public LocalBSDF
    {
        Spectrum C_;
        Spectrum Ctint_;

        real metallic_;
        real roughness_;
        real specular_tint_;
        real anisotropic_;
        real sheen_;
        real sheen_tint_;
        real clearcoat_;

        bool has_subsurface_; // 为true时diffuse为0，且transmission上多加一个specular transmission

        real transmission_;
        real IOR_; // inner IOR / outer IOR

        real ax_, ay_;
        real clearcoat_roughness_;

        static constexpr real SS_TRANS_ROUGH = real(0.01);
        struct SampleWeights
        {
            real diffuse          = real(0.25);
            real specular         = real(0.25);
            real clearcoat        = real(0.25);
            real transmission     = real(0.25);
            real subsurface_trans = real(0.25);
        } sample_w;

        static real one_minus_5(real x) noexcept
        {
            real t = 1 - x;
            real t2 = t * t;
            return t2 * t2 * t;
        }

        static real eta_to_R0(real eta) noexcept
        {
            return sqr(eta - 1) / sqr(eta + 1);
        }

        static Spectrum schlick(const Spectrum &R0, real cos_theta) noexcept
        {
            return R0 + (Spectrum(1) - R0) * one_minus_5(cos_theta);
        }

        static real schlick(const real &R0, real cos_theta) noexcept
        {
            return R0 + (1 - R0) * one_minus_5(cos_theta);
        }

        static Spectrum to_tint(const Spectrum &base_color) noexcept
        {
            real lum = base_color.lum();
            return lum > 0 ? base_color / lum : Spectrum(1);
        }

        Spectrum f_diffuse(real cos_theta_i, real cos_theta_o, real cos_theta_d) const noexcept
        {
            if(has_subsurface_)
                return {};

            Spectrum f_lambert = C_ / PI_r;

            real FL = one_minus_5(cos_theta_i);
            real FV = one_minus_5(cos_theta_o);
            real RR = 2 * roughness_ * cos_theta_d * cos_theta_d;
            Spectrum F_retro_refl = C_ / PI_r * RR * (FL + FV + FL * FV * (RR - 1));

            return f_lambert * (1 - real(0.5) * FL) * (1 - real(0.5) * FV) + F_retro_refl;
        }

        Spectrum f_sheen(real cos_theta_d) const noexcept
        {
            return 4 * sheen_ * mix(Spectrum(1), Ctint_, sheen_tint_) * one_minus_5(cos_theta_d);
        }

        Spectrum f_clearcoat(
            real cos_theta_i, real cos_theta_o,
            real tan_theta_i, real tan_theta_o,
            real sin_theta_h, real cos_theta_h, real cos_theta_d) const noexcept
        {
            assert(cos_theta_i > 0 && cos_theta_o > 0);
            real D = microfacet::gtr1(sin_theta_h, cos_theta_h, clearcoat_roughness_);
            real F = schlick(real(0.04), cos_theta_d);
            real G = microfacet::smith_gtr2(tan_theta_i, real(0.25))
                   * microfacet::smith_gtr2(tan_theta_o, real(0.25));
            return Spectrum(clearcoat_ * D * F * G / std::abs(4 * cos_theta_i * cos_theta_o));
        }

        Spectrum f_trans(const Vec3 &lwi, const Vec3 &lwo, TransportMode mode) const noexcept
        {
            assert(lwi.z * lwo.z < 0);

            if(has_subsurface_)
                return {};

            real cos_theta_i = local_angle::cos_theta(lwi);
            real cos_theta_o = local_angle::cos_theta(lwo);

            real eta = cos_theta_o > 0 ? IOR_ : 1 / IOR_;
            Vec3 lwh = (lwo + eta * lwi).normalize();
            if(lwh.z < 0)
                lwh = -lwh;

            real cos_theta_d = dot(lwo, lwh);
            real F = refl_aux::dielectric_fresnel(IOR_, 1, cos_theta_d);

            real phi_h       = local_angle::phi(lwh);
            real sin_phi_h   = std::sin(phi_h);
            real cos_phi_h   = std::cos(phi_h);
            real cos_theta_h = local_angle::cos_theta(lwh);
            real sin_theta_h = local_angle::cos_2_sin(cos_theta_h);
            real D = microfacet::anisotropic_gtr2(sin_phi_h, cos_phi_h, sin_theta_h, cos_theta_h, ax_, ay_);

            real phi_i       = local_angle::phi(lwi);
            real phi_o       = local_angle::phi(lwo);
            real sin_phi_i   = std::sin(phi_i), cos_phi_i = std::cos(phi_i);
            real sin_phi_o   = std::sin(phi_o), cos_phi_o = std::cos(phi_o);
            real tan_theta_i = local_angle::tan_theta(lwi);
            real tan_theta_o = local_angle::tan_theta(lwo);
            real G = microfacet::smith_anisotropic_gtr2(cos_phi_i, sin_phi_i, ax_, ay_, tan_theta_i)
                   * microfacet::smith_anisotropic_gtr2(cos_phi_o, sin_phi_o, ax_, ay_, tan_theta_o);

            real sdem = cos_theta_d + eta * dot(lwi, lwh);
            real corr_factor = mode == TM_Radiance ? (1 / eta) : 1;

            real(*std_sqrt)(real) = std::sqrt;
            Spectrum sqrtC = C_.map(std_sqrt);

            real val = (1 - F) * D * G * eta * eta * dot(lwi, lwh) * dot(lwo, lwh)
                     * corr_factor * corr_factor
                     / (cos_theta_i * cos_theta_o * sdem * sdem);

            real trans_factor = cos_theta_o > 0 ? transmission_ : 1;
            return (1 - metallic_) * trans_factor * sqrtC * std::abs(val);
        }

        Spectrum f_subsurface_trans(const Vec3 &lwi, const Vec3 &lwo, TransportMode mode) const
        {
            assert(lwo.z > 0 && lwi.z < 0);
            if(!has_subsurface_)
                return {};

            real cos_theta_i = local_angle::cos_theta(lwi);
            real cos_theta_o = local_angle::cos_theta(lwo);

            real eta = IOR_;
            Vec3 lwh = (lwo + eta * lwi).normalize();
            if(lwh.z < 0)
                lwh = -lwh;

            real cos_theta_d = dot(lwo, lwh);
            real F = refl_aux::dielectric_fresnel(IOR_, 1, cos_theta_d);

            real cos_theta_h = local_angle::cos_theta(lwh);
            real D = microfacet::gtr2(cos_theta_h, SS_TRANS_ROUGH);
            
            real tan_theta_i = local_angle::tan_theta(lwi);
            real tan_theta_o = local_angle::tan_theta(lwo);
            real G = microfacet::smith_gtr2(tan_theta_i, SS_TRANS_ROUGH)
                   * microfacet::smith_gtr2(tan_theta_o, SS_TRANS_ROUGH);

            real sdem = cos_theta_d + eta * dot(lwi, lwh);
            real corr_factor = mode == TM_Radiance ? (1 / eta) : 1;

            real val = (1 - F) * D * G * eta * eta * dot(lwi, lwh) * dot(lwo, lwh)
                * corr_factor * corr_factor
                / (cos_theta_i * cos_theta_o * sdem * sdem);

            return (1 - metallic_) * (1 - transmission_) * C_ * std::abs(val);
        }

        Spectrum f_inner_refl(const Vec3 &lwi, const Vec3 &lwo) const noexcept
        {
            assert(lwi.z < 0 && lwo.z < 0);
            if(has_subsurface_)
                return {};

            Vec3 lwh = -(lwi + lwo).normalize();
            assert(lwh.z > 0);

            real cos_theta_d = dot(lwo, lwh);
            real F = refl_aux::dielectric_fresnel(IOR_, 1, cos_theta_d);
            
            real phi_h       = local_angle::phi(lwh);
            real sin_phi_h   = std::sin(phi_h);
            real cos_phi_h   = std::cos(phi_h);
            real cos_theta_h = local_angle::cos_theta(lwh);
            real sin_theta_h = local_angle::cos_2_sin(cos_theta_h);
            real D = microfacet::anisotropic_gtr2(sin_phi_h, cos_phi_h, sin_theta_h, cos_theta_h, ax_, ay_);

            real phi_i       = local_angle::phi(lwi);
            real phi_o       = local_angle::phi(lwo);
            real sin_phi_i   = std::sin(phi_i), cos_phi_i = std::cos(phi_i);
            real sin_phi_o   = std::sin(phi_o), cos_phi_o = std::cos(phi_o);
            real tan_theta_i = local_angle::tan_theta(lwi);
            real tan_theta_o = local_angle::tan_theta(lwo);
            real G = microfacet::smith_anisotropic_gtr2(cos_phi_i, sin_phi_i, ax_, ay_, tan_theta_i)
                   * microfacet::smith_anisotropic_gtr2(cos_phi_o, sin_phi_o, ax_, ay_, tan_theta_o);

            return transmission_ * C_ * std::abs(F * D * G / (4 * lwi.z * lwo.z));
        }

        Spectrum f_specular(const Vec3 &lwi, const Vec3 &lwo) const noexcept
        {
            assert(lwi.z > 0 && lwo.z > 0);

            real cos_theta_i = local_angle::cos_theta(lwi);
            real cos_theta_o = local_angle::cos_theta(lwo);

            Vec3 lwh = (lwi + lwo).normalize();
            real cos_theta_d = dot(lwi, lwh);

            Spectrum Cspec = mix(
                mix(Spectrum(1), Ctint_, specular_tint_),
                C_, metallic_);
            Spectrum dielectric_fresnel = Cspec * refl_aux::dielectric_fresnel(IOR_, 1, cos_theta_d);
            Spectrum conductor_fresnel = schlick(Cspec, cos_theta_d);
            Spectrum F = mix(dielectric_fresnel, conductor_fresnel, metallic_);
            
            real phi_h       = local_angle::phi(lwh);
            real sin_phi_h   = std::sin(phi_h);
            real cos_phi_h   = std::cos(phi_h);
            real cos_theta_h = local_angle::cos_theta(lwh);
            real sin_theta_h = local_angle::cos_2_sin(cos_theta_h);
            real D = microfacet::anisotropic_gtr2(sin_phi_h, cos_phi_h, sin_theta_h, cos_theta_h, ax_, ay_);
            
            real phi_i       = local_angle::phi(lwi);
            real phi_o       = local_angle::phi(lwo);
            real sin_phi_i   = std::sin(phi_i), cos_phi_i = std::cos(phi_i);
            real sin_phi_o   = std::sin(phi_o), cos_phi_o = std::cos(phi_o);
            real tan_theta_i = local_angle::tan_theta(lwi);
            real tan_theta_o = local_angle::tan_theta(lwo);
            real G = microfacet::smith_anisotropic_gtr2(cos_phi_i, sin_phi_i, ax_, ay_, tan_theta_i)
                   * microfacet::smith_anisotropic_gtr2(cos_phi_o, sin_phi_o, ax_, ay_, tan_theta_o);

            return F * D * G / std::abs(4 * cos_theta_i * cos_theta_o);
        }

        Vec3 sample_diffuse(const Sample2 &sam) const noexcept
        {
            if(has_subsurface_)
                return {};
            return math::distribution::zweighted_on_hemisphere(sam.u, sam.v).first;
        }

        Vec3 sample_specular(const Vec3 &lwo, const Sample2 &sam) const noexcept
        {
            Vec3 lwh = microfacet::sample_anisotropic_gtr2(ax_, ay_, sam).normalize();
            if(lwh.z <= 0)
                return {};

            Vec3 lwi = (2 * dot(lwo, lwh) * lwh - lwo).normalize();
            if(lwi.z <= 0)
                return {};

            return lwi;
        }

        Vec3 sample_clearcoat(const Vec3 &lwo, const Sample2 &sam) const noexcept
        {
            Vec3 lwh = microfacet::sample_gtr1(clearcoat_roughness_, sam);
            if(lwh.z <= 0)
                return {};

            Vec3 lwi = (2 * dot(lwo, lwh) * lwh - lwo).normalize();
            if(lwi.z <= 0)
                return {};

            return lwi;
        }

        Vec3 sample_transmission(const Vec3 &lwo, const Sample2 &sam) const noexcept
        {
            if(has_subsurface_)
                return {};

            Vec3 lwh = microfacet::sample_anisotropic_gtr2(ax_, ay_, sam);
            if(lwh.z <= 0)
                return {};

            if((lwo.z > 0) != (dot(lwh, lwo) > 0))
                return {};

            real eta = lwo.z > 0 ? 1 / IOR_ : IOR_;
            Vec3 owh = dot(lwh, lwo) > 0 ? lwh : -lwh;
            auto opt_lwi = refl_aux::refract(lwo, owh, eta);
            if(!opt_lwi)
                return {};

            Vec3 lwi = opt_lwi->normalize();
            if(lwi.z * lwo.z > 0 || ((lwi.z > 0) != (dot(lwh, lwi) > 0)))
                return {};

            return lwi;
        }

        Vec3 sample_subsurface_trans(const Vec3 &lwo, const Sample2 &sam) const noexcept
        {
            if(!has_subsurface_ || lwo.z < 0)
                return {};

            Vec3 lwh = microfacet::sample_gtr2(SS_TRANS_ROUGH, sam);
            if(lwh.z <= 0)
                return {};

            if((lwo.z > 0) != (dot(lwh, lwo) > 0))
                return {};

            real eta = 1 / IOR_;
            Vec3 owh = dot(lwh, lwo) > 0 ? lwh : -lwh;
            auto opt_lwi = refl_aux::refract(lwo, owh, eta);
            if(!opt_lwi)
                return {};

            Vec3 lwi = opt_lwi->normalize();
            if(lwi.z * lwo.z > 0 || ((lwi.z > 0) != (dot(lwh, lwi) > 0)))
                return {};

            return lwi;
        }

        Vec3 sample_inner_refl(const Vec3 &lwo, const Sample2 &sam) const noexcept
        {
            assert(lwo.z < 0);
            if(has_subsurface_)
                return {};

            Vec3 lwh = microfacet::sample_anisotropic_gtr2(ax_, ay_, sam);
            if(lwh.z <= 0)
                return {};

            Vec3 lwi = (2 * dot(lwo, lwh) * lwh - lwo);
            if(lwi.z > 0)
                return {};
            return lwi.normalize();
        }

        real pdf_diffuse(const Vec3 &lwi, const Vec3 &lwo) const noexcept
        {
            assert(lwi.z > 0 && lwo.z > 0);
            return math::distribution::zweighted_on_hemisphere_pdf(lwi.z);
        }

        std::pair<real, real> pdf_specular_clearcoat(const Vec3 &lwi, const Vec3 &lwo) const noexcept
        {
            assert(lwi.z > 0 && lwo.z > 0);

            Vec3 lwh = (lwi + lwo).normalize();
            real phi_h       = local_angle::phi(lwh);
            real sin_phi_h   = std::sin(phi_h);
            real cos_phi_h   = std::cos(phi_h);
            real cos_theta_h = local_angle::cos_theta(lwh);
            real sin_theta_h = local_angle::cos_2_sin(cos_theta_h);
            real cos_theta_d = dot(lwi, lwh);
            
            real specular_D = microfacet::anisotropic_gtr2(sin_phi_h, cos_phi_h, sin_theta_h, cos_theta_h, ax_, ay_);
            real pdf_specular = cos_theta_h * specular_D / (4 * cos_theta_d);

            real clearcoat_D = microfacet::gtr1(sin_theta_h, cos_theta_h, clearcoat_roughness_);
            real pdf_clearcoat = cos_theta_h * clearcoat_D / (4 * cos_theta_d);

            return { pdf_specular, pdf_clearcoat };
        }

        real pdf_transmission(const Vec3 &lwi, const Vec3 &lwo) const noexcept
        {
            assert(lwi.z * lwo.z < 0);

            if(has_subsurface_)
                return 0;

            real eta = lwo.z > 0 ? IOR_ : 1 / IOR_;
            Vec3 lwh = (lwo + eta * lwi).normalize();
            if(lwh.z < 0)
                lwh = -lwh;

            if(((lwo.z > 0) != (dot(lwh, lwo) > 0)) ||
               ((lwi.z > 0) != (dot(lwh, lwi) > 0)))
                return 0;

            real sdem = dot(lwo, lwh) + eta * dot(lwi, lwh);
            real dwh_to_dwi = eta * eta * dot(lwi, lwh) / (sdem * sdem);

            real phi_h       = local_angle::phi(lwh);
            real sin_phi_h   = std::sin(phi_h);
            real cos_phi_h   = std::cos(phi_h);
            real cos_theta_h = local_angle::cos_theta(lwh);
            real sin_theta_h = local_angle::cos_2_sin(cos_theta_h);

            real D = microfacet::anisotropic_gtr2(sin_phi_h, cos_phi_h, sin_theta_h, cos_theta_h, ax_, ay_);
            return std::abs(dot(lwi, lwh) * D * dwh_to_dwi);
        }

        real pdf_subsurface_trans(const Vec3 &lwi, const Vec3 &lwo) const noexcept
        {
            assert(lwo.z > 0 && lwi.z < 0);
            if(!has_subsurface_)
                return 0;

            real eta = IOR_;
            Vec3 lwh = (lwo + eta * lwi).normalize();
            if(lwh.z < 0)
                lwh = -lwh;

            if(((lwo.z > 0) != (dot(lwh, lwo) > 0)) ||
               ((lwi.z > 0) != (dot(lwh, lwi) > 0)))
                return 0;

            real sdem = dot(lwo, lwh) + eta * dot(lwi, lwh);
            real dwh_to_dwi = eta * eta * dot(lwi, lwh) / (sdem * sdem);

            real cos_theta_h = local_angle::cos_theta(lwh);
            real D = microfacet::gtr2(cos_theta_h, SS_TRANS_ROUGH);
            return std::abs(dot(lwi, lwh) * D * dwh_to_dwi);
        }

        real pdf_inner_refl(const Vec3 &lwi, const Vec3 &lwo) const noexcept
        {
            assert(lwi.z < 0 && lwo.z < 0);
            if(has_subsurface_)
                return {};

            Vec3 lwh = -(lwi + lwo).normalize();
            real phi_h       = local_angle::phi(lwh);
            real sin_phi_h   = std::sin(phi_h);
            real cos_phi_h   = std::cos(phi_h);
            real cos_theta_h = local_angle::cos_theta(lwh);
            real sin_theta_h = local_angle::cos_2_sin(cos_theta_h);
            real cos_theta_d = dot(lwi, lwh);
            
            real D = microfacet::anisotropic_gtr2(sin_phi_h, cos_phi_h, sin_theta_h, cos_theta_h, ax_, ay_);
            return std::abs(cos_theta_h * D / (4 * cos_theta_d));
        }

    public:

        DisneyBSDF(const Coord &geometry_coord, const Coord &shading_coord,
                   const Spectrum &base_color,
                   real metallic,
                   real roughness,
                   real specular_tint,
                   real anisotropic,
                   real sheen,
                   real sheen_tint,
                   real clearcoat,
                   real clearcoat_gloss,
                   real transmission,
                   real IOR,
                   bool has_subsurface)
            : LocalBSDF(geometry_coord, shading_coord)
        {
            C_     = base_color;
            Ctint_ = to_tint(base_color);

            metallic_      = metallic;
            roughness_     = roughness;
            specular_tint_ = specular_tint;
            anisotropic_   = anisotropic;
            sheen_         = sheen;
            sheen_tint_    = sheen_tint;

            transmission_  = transmission;
            IOR_           = IOR;

            has_subsurface_ = has_subsurface;

            real aspect = anisotropic > 0 ? std::sqrt(1 - real(0.9) * anisotropic) : real(1);
            ax_ = std::max(real(0.001), sqr(roughness) / aspect);
            ay_ = std::max(real(0.001), sqr(roughness) * aspect);

            clearcoat_ = clearcoat;
            clearcoat_roughness_ = mix(real(0.1), real(0.01), clearcoat_gloss);

            real A = (std::min)(real(0.8), 1 - metallic_);
            real B = 1 - A;

            if(!has_subsurface)
            {
                sample_w.diffuse          = A * (1 - transmission_);
                sample_w.transmission     = A * transmission_;
                sample_w.subsurface_trans = 0;
            }
            else
            {
                sample_w.diffuse          = 0;
                sample_w.transmission     = A * transmission_;
                sample_w.subsurface_trans = A * (1 - transmission_);
            }
            sample_w.specular     = B * 2 / (2 + clearcoat_);
            sample_w.clearcoat    = B * clearcoat_ / (2 + clearcoat_);
        }

        Spectrum eval(const Vec3 &wi, const Vec3 &wo, TransportMode mode) const noexcept override
        {
            if(cause_black_fringes(wi, wo))
                return eval_for_black_fringes(wi, wo);

            const Vec3 lwi = shading_coord_.global_to_local(wi).normalize();
            const Vec3 lwo = shading_coord_.global_to_local(wo).normalize();

            if(std::abs(lwi.z) < EPS || std::abs(lwo.z) < EPS)
                return {};

            // transmission
            
            if(lwi.z * lwo.z < 0)
            {
                if(!transmission_ && !has_subsurface_)
                    return {};
                Spectrum value = f_trans(lwi, lwo, mode);
                if(has_subsurface_ && lwo.z > 0)
                    value += f_subsurface_trans(lwi, lwo, mode);
                return value * local_angle::normal_corr_factor(geometry_coord_, shading_coord_, wi);
            }

            // inner refl

            if(lwi.z < 0 && lwo.z < 0)
            {
                if(!transmission_)
                    return {};
                Spectrum value = f_inner_refl(lwi, lwo);
                return value * local_angle::normal_corr_factor(geometry_coord_, shading_coord_, wi);
            }

            // reflection

            if(lwi.z <= 0 || lwo.z <= 0)
                return {};

            real cos_theta_i = local_angle::cos_theta(lwi);
            real cos_theta_o = local_angle::cos_theta(lwo);

            Vec3 lwh = (lwi + lwo).normalize();
            real cos_theta_d = dot(lwi, lwh);

            Spectrum diffuse, sheen;
            if(metallic_ < 1)
            {
                diffuse = f_diffuse(cos_theta_i, cos_theta_o, cos_theta_d);
                if(sheen_ > 0)
                    sheen = f_sheen(cos_theta_d);
            }

            Spectrum specular = f_specular(lwi, lwo);

            Spectrum clearcoat;
            if(clearcoat_ > 0)
            {
                real tan_theta_i = local_angle::tan_theta(lwi);
                real tan_theta_o = local_angle::tan_theta(lwo);
                real cos_theta_h = local_angle::cos_theta(lwh);
                real sin_theta_h = local_angle::cos_2_sin(cos_theta_h);
                clearcoat = f_clearcoat(cos_theta_i, cos_theta_o, tan_theta_i, tan_theta_o,
                                        sin_theta_h, cos_theta_h, cos_theta_d);
            }

            Spectrum value = (1 - metallic_) * (1 - transmission_) * (diffuse + sheen) + specular + clearcoat;
            return value * local_angle::normal_corr_factor(geometry_coord_, shading_coord_, wi);
        }

        BSDFSampleResult sample(const Vec3 &wo, TransportMode mode, const Sample3 &sam) const noexcept override
        {
            if(cause_black_fringes(wo))
                return sample_for_black_fringes(wo, mode, sam);

            Vec3 lwo = shading_coord_.global_to_local(wo).normalize();
            if(std::abs(lwo.z) < EPS)
                return BSDF_SAMPLE_RESULT_INVALID;

            // transmission and inner refl

            if(lwo.z < 0)
            {
                if(!transmission_)
                    return BSDF_SAMPLE_RESULT_INVALID;

                Vec3 lwi;
                real macro_F = refl_aux::dielectric_fresnel(IOR_, 1, lwo.z);
                macro_F = math::clamp(macro_F, real(0.1), real(0.9));
                if(sam.u >= macro_F)
                    lwi = sample_transmission(lwo, { sam.v, sam.w });
                else
                    lwi = sample_inner_refl(lwo, { sam.v, sam.w });

                if(!lwi)
                    return BSDF_SAMPLE_RESULT_INVALID;

                BSDFSampleResult ret;
                ret.dir      = shading_coord_.local_to_global(lwi);
                ret.f        = eval(ret.dir, wo, mode);
                ret.pdf      = pdf(ret.dir, wo, mode);
                ret.mode     = mode;
                ret.is_delta = false;

                if(!ret.f.is_finite() || ret.pdf < EPS)
                    return BSDF_SAMPLE_RESULT_INVALID;

                return ret;
            }

            // reflection + transmission

            real sam_selector = sam.u;
            Sample2 new_sam{ sam.v, sam.w };

            Vec3 lwi;
            if(sam_selector < sample_w.diffuse)
                lwi = sample_diffuse(new_sam);
            else if(sam_selector -= sample_w.diffuse; sam_selector < sample_w.transmission)
                lwi = sample_transmission(lwo, new_sam);
            else if(sam_selector -= sample_w.transmission; sam_selector < sample_w.subsurface_trans)
                lwi = sample_subsurface_trans(lwo, new_sam);
            else if(sam_selector -= sample_w.subsurface_trans; sam_selector < sample_w.specular)
                lwi = sample_specular(lwo, new_sam);
            else
                lwi = sample_clearcoat(lwo, new_sam);

            if(!lwi)
                return BSDF_SAMPLE_RESULT_INVALID;
            
            BSDFSampleResult ret;
            ret.dir      = shading_coord_.local_to_global(lwi);
            ret.f        = eval(ret.dir, wo, mode);
            ret.pdf      = pdf(ret.dir, wo, mode);
            ret.mode     = mode;
            ret.is_delta = false;

            if(!ret.f.is_finite() || ret.pdf < EPS)
                return BSDF_SAMPLE_RESULT_INVALID;

            return ret;
        }

        real pdf(const Vec3 &wi, const Vec3 &wo, TransportMode mode) const noexcept override
        {
            if(cause_black_fringes(wi, wo))
                return pdf_for_black_fringes(wi, wo);

            Vec3 lwi = shading_coord_.global_to_local(wi).normalize();
            Vec3 lwo = shading_coord_.global_to_local(wo).normalize();
            if(std::abs(lwi.z) < EPS || std::abs(lwo.z) < EPS)
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

            real subsurface_trans = 0;
            if(has_subsurface_ && lwi.z < 0)
                subsurface_trans = pdf_subsurface_trans(lwi, lwo);

            if(lwi.z < 0)
            {
                return sample_w.subsurface_trans * subsurface_trans
                     + sample_w.transmission * pdf_transmission(lwi, lwo);
            }

            real diffuse = pdf_diffuse(lwi, lwo);
            auto [specular, clearcoat] = pdf_specular_clearcoat(lwi, lwo);
            return sample_w.diffuse   * diffuse
                 + sample_w.specular  * specular
                 + sample_w.clearcoat * clearcoat;
        }

        Spectrum albedo() const noexcept override
        {
            return C_;
        }

        bool is_delta() const noexcept override
        {
            return false;
        }

        bool is_black() const noexcept override
        {
            return false;
        }
    };

} // namespace disney_impl

class Disney : public Material
{
    const Texture *base_color_       = nullptr;
    const Texture *metallic_         = nullptr;
    const Texture *roughness_        = nullptr;
    const Texture *specular_tint_    = nullptr;
    const Texture *anisotropic_      = nullptr;
    const Texture *sheen_            = nullptr;
    const Texture *sheen_tint_       = nullptr;
    const Texture *clearcoat_        = nullptr;
    const Texture *clearcoat_gloss_  = nullptr;
    const Texture *transmission_     = nullptr;
    const Texture *IOR_              = nullptr;
    const Texture *scatter_distance_ = nullptr;

    NormalMapper normal_mapper_;

public:

    using Material::Material;

    static std::string description()
    {
        return R"___(
disney [Material]
    base_color       [Texture]
    metallic         [Texture]
    roughness        [Texture]
    transmission     [Texture] (optional; defaultly set to all_zero)
    ior              [Texture] (optional; defaultly set to all_{1.5})
    specular_tint    [Texture] (optional; defaultly set to all_zero)
    anisotropic      [Texture] (optional; defaultly set to all_zero)
    sheen            [Texture] (optional; defaultly set to all_zero)
    sheen_tint       [Texture] (optional; defaultly set to all_zero)
    clearcoat        [Texture] (optional; defaultly set to all_zero)
    clearcoat_gloss  [Texture] (optional; defaultly set to all_{0.5})

    disney principled bsdf
    see https://blog.selfshadow.com/publications/s2015-shading-course/#course_content
)___";
    }

    void initialize(const Config &params, obj::ObjectInitContext &init_ctx) override
    {
        AGZ_HIERARCHY_TRY

        auto defaultly_all = [&](const char *name, real default_value)
        {
            if(auto node = params.find_child_group(name))
                return TextureFactory.create(*node, init_ctx);
            
            ConfigGroup group;
            group.insert_child("type", std::make_shared<ConfigValue>("constant"));

            auto arr = std::make_shared<ConfigArray>();
            arr->push_back(std::make_shared<ConfigValue>(std::to_string(default_value)));
            group.insert_child("texel", std::move(arr));

            return TextureFactory.create(group, init_ctx);
        };

        base_color_       = TextureFactory.create(params.child_group("base_color"), init_ctx);
        metallic_         = TextureFactory.create(params.child_group("metallic"), init_ctx);
        roughness_        = TextureFactory.create(params.child_group("roughness"), init_ctx);
        transmission_     = defaultly_all("transmission", 0);
        IOR_              = defaultly_all("ior", real(1.5));
        specular_tint_    = defaultly_all("specular_tint", 0);
        anisotropic_      = defaultly_all("anisotropic", 0);
        sheen_            = defaultly_all("sheen", 0);
        sheen_tint_       = defaultly_all("sheen_tint", 0);
        clearcoat_        = defaultly_all("clearcoat", 0);
        clearcoat_gloss_  = defaultly_all("clearcoat_gloss", real(0.5));
        scatter_distance_ = defaultly_all("scatter_distance", 0);

        normal_mapper_.initialize(params, init_ctx);

        AGZ_HIERARCHY_WRAP("in initializing disney principled material")
    }

    ShadingPoint shade(const EntityIntersection &inct, Arena &arena) const override
    {
        Vec2 uv = inct.uv;
        Spectrum base_color       = base_color_      ->sample_spectrum(uv);
        real     metallic         = metallic_        ->sample_real(uv);
        real     roughness        = roughness_       ->sample_real(uv);
        real     transmission     = transmission_    ->sample_real(uv);
        real     ior              = IOR_             ->sample_real(uv);
        real     specular_tint    = specular_tint_   ->sample_real(uv);
        real     anisotropic      = anisotropic_     ->sample_real(uv);
        real     sheen            = sheen_           ->sample_real(uv);
        real     sheen_tint       = sheen_tint_      ->sample_real(uv);
        real     clearcoat        = clearcoat_       ->sample_real(uv);
        real     clearcoat_gloss  = clearcoat_gloss_ ->sample_real(uv);
        Spectrum scatter_distance = scatter_distance_->sample_spectrum(uv);
        bool has_subsurface = scatter_distance.lum() > 0;

        Coord shading_coord = normal_mapper_.reorient(uv, inct.user_coord);
        auto bsdf = arena.create<disney_impl::DisneyBSDF>(
            inct.geometry_coord, shading_coord,
            base_color,
            metallic,
            roughness,
            specular_tint,
            anisotropic,
            sheen,
            sheen_tint,
            clearcoat,
            clearcoat_gloss,
            transmission,
            ior,
            has_subsurface);

        ShadingPoint shd = { bsdf };
        if(has_subsurface)
        {
            shd.bssrdf = arena.create<NormalizedDiffusionBSSRDF>(
                inct, inct.geometry_coord, inct.geometry_coord,
                ior, base_color * (1 - metallic) * (1 - transmission), scatter_distance);
        }

        return shd;
    }
};

AGZT_IMPLEMENTATION(Material, Disney, "disney")

AGZ_TRACER_END
