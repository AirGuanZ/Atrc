#include "../utility/microfacet.h"
#include "./microfacet_refl_comp.h"

AGZ_TRACER_BEGIN

namespace
{
    template<bool EVAL, bool PDF>
    void eval_and_pdf(
        const FVec3 &lwi, const FVec3 &lwo,
        const FresnelPoint *fresnel, real ax, real ay,
        Spectrum *eval_output, real *pdf_output) noexcept
    {
        if(lwi.z <= 0 || lwo.z <= 0)
        {
            if constexpr(EVAL)
                *eval_output = {};
            if constexpr(PDF)
                *pdf_output = 0;
            return;
        }

        const real cos_theta_i = local_angle::cos_theta(lwi);
        const real cos_theta_o = local_angle::cos_theta(lwo);

        const FVec3 lwh = (lwi + lwo).normalize();
        const real cos_theta_d = dot(lwi, lwh);

        const real phi_h       = local_angle::phi(lwh);
        const real sin_phi_h   = std::sin(phi_h);
        const real cos_phi_h   = std::cos(phi_h);
        const real cos_theta_h = local_angle::cos_theta(lwh);
        const real sin_theta_h = local_angle::cos_2_sin(cos_theta_h);
        const real D = microfacet::anisotropic_gtr2(
            sin_phi_h, cos_phi_h, sin_theta_h, cos_theta_h, ax, ay);

        const real phi_i       = local_angle::phi(lwi);
        const real phi_o       = local_angle::phi(lwo);
        const real sin_phi_i   = std::sin(phi_i), cos_phi_i = std::cos(phi_i);
        const real sin_phi_o   = std::sin(phi_o), cos_phi_o = std::cos(phi_o);
        const real tan_theta_i = local_angle::tan_theta(lwi);
        const real tan_theta_o = local_angle::tan_theta(lwo);

        const real Go = microfacet::smith_anisotropic_gtr2(
                            cos_phi_o, sin_phi_o, ax, ay, tan_theta_o);

        if constexpr(EVAL)
        {
            const real Gi = microfacet::smith_anisotropic_gtr2(
                cos_phi_i, sin_phi_i, ax, ay, tan_theta_i);
            const real G = Gi * Go;

            const Spectrum F = fresnel->eval(cos_theta_d);

            *eval_output = F * D * G / std::abs(4 * cos_theta_i * cos_theta_o);
        }

        if constexpr(PDF)
        {
            *pdf_output = Go * D / (4 * lwo.z);
        }
    }
}

GGXMicrofacetReflectionComponent::GGXMicrofacetReflectionComponent(
    const FresnelPoint *fresnel,
    real roughness, real anisotropic) noexcept
    : BSDFComponent(BSDF_GLOSSY)
{
    fresnel_ = fresnel;

    const real aspect = anisotropic > 0 ?
        std::sqrt(1 - real(0.9) * anisotropic) : real(1);
    ax_ = std::max(real(0.001), math::sqr(roughness) / aspect);
    ay_ = std::max(real(0.001), math::sqr(roughness) * aspect);
}

Spectrum GGXMicrofacetReflectionComponent::eval(
    const FVec3 &lwi, const FVec3 &lwo, TransMode mode) const noexcept
{
    Spectrum ret;
    eval_and_pdf<true, false>(lwi, lwo, fresnel_, ax_, ay_, &ret, nullptr);
    return ret;
}

BSDFComponent::SampleResult GGXMicrofacetReflectionComponent::sample(
    const FVec3 &lwo, TransMode mode, const Sample2 &sam) const noexcept
{
    if(lwo.z <= 0)
        return {};

    const FVec3 lwh = microfacet::sample_anisotropic_gtr2_vnor(
        lwo, ax_, ay_, sam).normalize();
    if(lwh.z <= 0)
        return {};

    const FVec3 lwi = (2 * dot(lwo, lwh) * lwh - lwo).normalize();
    if(lwi.z <= 0)
        return {};

    SampleResult ret;
    ret.lwi = lwi;
    eval_and_pdf<true, true>(lwi, lwo, fresnel_, ax_, ay_, &ret.f, &ret.pdf);

    return ret;
}

real GGXMicrofacetReflectionComponent::pdf(
    const FVec3 &lwi, const FVec3 &lwo) const noexcept
{
    real ret;
    eval_and_pdf<false, true>(lwi, lwo, fresnel_, ax_, ay_, nullptr, &ret);
    return ret;
}

AGZ_TRACER_END
