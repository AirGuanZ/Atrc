#include <agz/tracer/utility/reflection.h>

#include "../utility/microfacet.h"
#include "./microfacet_refr_comp.h"

AGZ_TRACER_BEGIN

FVec3 GGXMicrofacetRefractionComponent::sample_trans(
    const FVec3 &lwo, const Sample2 &sam) const noexcept
{
    const FVec3 lwh = microfacet::sample_anisotropic_gtr2(
                                        ax_, ay_, sam);
    if(lwh.z <= 0)
        return {};

    if((lwo.z > 0) != (dot(lwh, lwo) > 0))
        return {};

    const real eta = lwo.z > 0 ? 1 / ior_ : ior_;
    const FVec3 owh = dot(lwh, lwo) > 0 ? lwh : -lwh;
    auto opt_lwi = refl_aux::refract(lwo, owh, eta);
    if(!opt_lwi)
        return {};

    const FVec3 lwi = opt_lwi->normalize();
    if(lwi.z * lwo.z > 0 || ((lwi.z > 0) != (dot(lwh, lwi) > 0)))
        return {};

    return lwi;
}

FVec3 GGXMicrofacetRefractionComponent::sample_inner_refl(
    const FVec3 &lwo, const Sample2 &sam) const noexcept
{
    assert(lwo.z < 0);

    const FVec3 lwh = microfacet::sample_anisotropic_gtr2(
                                        ax_, ay_, sam);
    if(lwh.z <= 0)
        return {};

    const FVec3 lwi = (2 * dot(lwo, lwh) * lwh - lwo);
    if(lwi.z > 0)
        return {};
    return lwi.normalize();
}

real GGXMicrofacetRefractionComponent::pdf_trans(
    const FVec3 &lwi, const FVec3 &lwo) const noexcept
{
    assert(lwi.z * lwo.z < 0);

    const real eta = lwo.z > 0 ? ior_ : 1 / ior_;
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
        sin_theta_h, cos_theta_h, ax_, ay_);
    return std::abs(dot(lwi, lwh) * D * dwh_to_dwi);
}

real GGXMicrofacetRefractionComponent::pdf_inner_refl(
    const FVec3 &lwi, const FVec3 &lwo) const noexcept
{
    assert(lwi.z < 0 && lwo.z < 0);

    const FVec3 lwh = -(lwi + lwo).normalize();
    const real phi_h = local_angle::phi(lwh);
    const real sin_phi_h = std::sin(phi_h);
    const real cos_phi_h = std::cos(phi_h);
    const real cos_theta_h = local_angle::cos_theta(lwh);
    const real sin_theta_h = local_angle::cos_2_sin(cos_theta_h);
    const real cos_theta_d = dot(lwi, lwh);

    const real D = microfacet::anisotropic_gtr2(
        sin_phi_h, cos_phi_h,
        sin_theta_h, cos_theta_h, ax_, ay_);
    return std::abs(cos_theta_h * D / (4 * cos_theta_d));
}

GGXMicrofacetRefractionComponent::GGXMicrofacetRefractionComponent(
    const Spectrum &color, real ior,
    real roughness, real anisotropic)
    : BSDFComponent(BSDF_GLOSSY)
{
    color_ = color;
    ior_ = ior;

    const real aspect = anisotropic > 0 ?
        std::sqrt(1 - real(0.9) * anisotropic) : real(1);
    ax_ = std::max(real(0.001), math::sqr(roughness) / aspect);
    ay_ = std::max(real(0.001), math::sqr(roughness) * aspect);
}

Spectrum GGXMicrofacetRefractionComponent::eval(
    const FVec3 &lwi, const FVec3 &lwo, TransMode mode) const noexcept
{
    // reflection

    if(lwi.z >= 0 && lwo.z >= 0)
        return {};

    // transmission

    if(lwi.z * lwo.z < 0)
    {
        const real cos_theta_i = local_angle::cos_theta(lwi);
        const real cos_theta_o = local_angle::cos_theta(lwo);

        const real eta = cos_theta_o > 0 ? ior_ : 1 / ior_;
        FVec3 lwh = (lwo + eta * lwi).normalize();
        if(lwh.z < 0)
            lwh = -lwh;

        const real cos_theta_d = dot(lwo, lwh);
        const real F = refl_aux::dielectric_fresnel(ior_, 1, cos_theta_d);

        const real phi_h       = local_angle::phi(lwh);
        const real sin_phi_h   = std::sin(phi_h);
        const real cos_phi_h   = std::cos(phi_h);
        const real cos_theta_h = local_angle::cos_theta(lwh);
        const real sin_theta_h = local_angle::cos_2_sin(cos_theta_h);
        const real D = microfacet::anisotropic_gtr2(
                            sin_phi_h, cos_phi_h,
                            sin_theta_h, cos_theta_h,
                            ax_, ay_);

        const real phi_i       = local_angle::phi(lwi);
        const real phi_o       = local_angle::phi(lwo);
        const real sin_phi_i   = std::sin(phi_i), cos_phi_i = std::cos(phi_i);
        const real sin_phi_o   = std::sin(phi_o), cos_phi_o = std::cos(phi_o);
        const real tan_theta_i = local_angle::tan_theta(lwi);
        const real tan_theta_o = local_angle::tan_theta(lwo);
        const real G = microfacet::smith_anisotropic_gtr2(
                            cos_phi_i, sin_phi_i,
                            ax_, ay_, tan_theta_i)
                     * microfacet::smith_anisotropic_gtr2(
                            cos_phi_o, sin_phi_o,
                            ax_, ay_, tan_theta_o);

        const real sdem = cos_theta_d + eta * dot(lwi, lwh);
        const real corr_factor = mode == TransMode::Radiance ? 1 / eta : 1;

        const real val = (1 - F) * D * G * eta * eta
                       * dot(lwi, lwh) * dot(lwo, lwh)
                       * corr_factor * corr_factor
                       / (cos_theta_i * cos_theta_o * sdem * sdem);

        return Spectrum(std::abs(val));
    }

    // inner reflection

    const FVec3 lwh = -(lwi + lwo).normalize();
    assert(lwh.z > 0);

    const real cos_theta_d = dot(lwo, lwh);
    const real F = refl_aux::dielectric_fresnel(ior_, 1, cos_theta_d);
    
    const real phi_h       = local_angle::phi(lwh);
    const real sin_phi_h   = std::sin(phi_h);
    const real cos_phi_h   = std::cos(phi_h);
    const real cos_theta_h = local_angle::cos_theta(lwh);
    const real sin_theta_h = local_angle::cos_2_sin(cos_theta_h);
    const real D = microfacet::anisotropic_gtr2(
                        sin_phi_h, cos_phi_h,
                        sin_theta_h, cos_theta_h,
                        ax_, ay_);

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
                        ax_, ay_, tan_theta_i)
                 * microfacet::smith_anisotropic_gtr2(
                        cos_phi_o, sin_phi_o,
                        ax_, ay_, tan_theta_o);

    return Spectrum(std::abs(F * D * G / (4 * lwi.z * lwo.z)));
}

BSDFComponent::SampleResult GGXMicrofacetRefractionComponent::sample(
    const FVec3 &lwo, TransMode mode, const Sample2 &sam) const noexcept
{
    if(lwo.z > 0)
    {
        // sample transmission

        const FVec3 lwi = sample_trans(lwo, sam);
        if(!lwi)
            return {};

        SampleResult ret;
        ret.f   = eval(lwi, lwo, mode);
        ret.lwi = lwi;
        ret.pdf = pdf(lwi, lwo);

        return ret;
    }

    // transmission/inner refl

    const real macro_F = math::clamp(
        refl_aux::dielectric_fresnel(ior_, 1, lwo.z),
        real(0.1), real(0.9));

    FVec3 lwi;
    if(sam.u >= macro_F)
    {
        const real new_u = (sam.u - macro_F) / (1 - macro_F);
        lwi = sample_trans(lwo, { new_u, sam.v });
    }
    else
    {
        const real new_u = sam.u / macro_F;
        lwi = sample_inner_refl(lwo, { new_u, sam.v });
    }

    if(!lwi)
        return {};

    SampleResult ret;
    ret.f   = eval(lwi, lwo, mode);
    ret.lwi = lwi;
    ret.pdf = pdf(lwi, lwo);

    return ret;
}

real GGXMicrofacetRefractionComponent::pdf(
    const FVec3 &lwi, const FVec3 &lwo) const noexcept
{
    if(lwi.z >= 0 && lwo.z >= 0)
        return 0;

    // transmission only

    if(lwo.z > 0)
        return pdf_trans(lwi, lwo);

    // transmission & inner_refl

    const real macro_F = math::clamp(
        refl_aux::dielectric_fresnel(ior_, 1, lwo.z),
        real(0.1), real(0.9));

    if(lwi.z > 0)
        return (1 - macro_F) * pdf_trans(lwi, lwo);
    return macro_F * pdf_inner_refl(lwi, lwo);
}

AGZ_TRACER_END
