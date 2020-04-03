#include "./diffuse_comp.h"

AGZ_TRACER_BEGIN

DiffuseComponent::DiffuseComponent(const Spectrum &albedo) noexcept
    : BSDFComponent(BSDF_REFLECTION | BSDF_DIFFUSE)
{
    coef_ = albedo / PI_r;
}

Spectrum DiffuseComponent::eval(
    const Vec3 &lwi, const Vec3 &lwo, TransMode mode) const noexcept
{
    if(lwi.z <= 0 || lwo.z <= 0)
        return {};
    return coef_;
}

real DiffuseComponent::pdf(
    const Vec3 &lwi, const Vec3 &lwo) const noexcept
{
    if(lwi.z <= 0 || lwo.z <= 0)
        return 0;
    return math::distribution::zweighted_on_hemisphere_pdf(lwi.z);
}

BSDFComponent::SampleResult DiffuseComponent::sample(
    const Vec3 &lwo, TransMode mode, const Sample2 &sam) const noexcept
{
    if(lwo.z <= 0)
        return {};

    auto [lwi, pdf] = math::distribution::zweighted_on_hemisphere(sam.u, sam.v);

    if(pdf < real(0.001))
        return {};

    SampleResult ret;
    ret.f   = coef_;
    ret.lwi = lwi;
    ret.pdf = pdf;

    return ret;
}

AGZ_TRACER_END
