#include "./diffuse_comp.h"

AGZ_TRACER_BEGIN

DiffuseComponent::DiffuseComponent(const FSpectrum &albedo) noexcept
{
    coef_ = albedo / PI_r;
}

FSpectrum DiffuseComponent::eval(const FVec3 &lwi, const FVec3 &lwo, TransMode mode) const
{
    if(lwi.z <= 0 || lwo.z <= 0)
        return {};
    return coef_;
}

real DiffuseComponent::pdf(const FVec3 &lwi, const FVec3 &lwo) const
{
    if(lwi.z <= 0 || lwo.z <= 0)
        return 0;
    return math::distribution::zweighted_on_hemisphere_pdf(lwi.z);
}

BSDFComponent::SampleResult DiffuseComponent::sample(const FVec3 &lwo, TransMode mode, const Sample2 &sam) const
{
    return discard_pdf_rev(sample_bidir(lwo, mode, sam));
}

BSDFComponent::BidirSampleResult DiffuseComponent::sample_bidir(
    const FVec3 &lwo, TransMode mode, const Sample2 &sam) const
{
    if(lwo.z <= 0)
        return {};

    auto [lwi, pdf] = math::distribution::zweighted_on_hemisphere(sam.u, sam.v);
    if(pdf < real(0.001))
        return {};

    BidirSampleResult ret;
    ret.f = coef_;
    ret.lwi = lwi;
    ret.pdf = pdf;
    ret.pdf_rev = math::distribution::zweighted_on_hemisphere_pdf(lwo.z);
    return ret;
}

bool DiffuseComponent::has_diffuse_component() const
{
    return true;
}

AGZ_TRACER_END
