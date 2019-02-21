#include <Atrc/Lib/Material/BxDF/BxDF_TorranceSparrow.h>

namespace Atrc
{
    
BxDF_TorranceSparrowReflection::BxDF_TorranceSparrowReflection(const Spectrum &rc, const MicrofacetDistribution *md, const Fresnel *fresnel) noexcept
    : BxDF(BSDFType(BSDF_REFLECTION | BSDF_GLOSSY)), rc_(rc), md_(md), fresnel_(fresnel)
{
    AGZ_ASSERT(md && fresnel);
}

Spectrum BxDF_TorranceSparrowReflection::GetAlbedo() const noexcept
{
    return rc_;
}

Spectrum BxDF_TorranceSparrowReflection::Eval(const Vec3 &wi, const Vec3 &wo, [[maybe_unused]] bool star) const noexcept
{
    if(wi.z <= 0 || wo.z <= 0)
        return Spectrum();

    Vec3 nWi = wi.Normalize(), nWo = wo.Normalize();
    Vec3 H = (nWi + nWo).Normalize();
    Real G = md_->G(H, nWi, nWo);
    if(!G)
        return Spectrum();
    Spectrum fr = fresnel_->Eval(Dot(nWi, H));
    return rc_ * fr * md_->D(H) * G / (4 * nWi.z * nWo.z);
}

std::optional<BxDF::SampleWiResult> BxDF_TorranceSparrowReflection::SampleWi(const Vec3 &wo, bool star, const Vec3 &sample) const noexcept
{
    Vec3 nWo = wo.Normalize();
    auto mdSample = md_->SampleWi(nWo, sample.xy());
    if(!mdSample)
        return std::nullopt;

    SampleWiResult ret;
    ret.coef = Eval(mdSample->wi, wo, star);
    if(!ret.coef)
        return std::nullopt;
    ret.wi      = mdSample->wi;
    ret.pdf     = mdSample->pdf;
    ret.type    = type_;
    ret.isDelta = false;

    return ret;
}

Real BxDF_TorranceSparrowReflection::SampleWiPDF(const Vec3 &wi, const Vec3 &wo, [[maybe_unused]] bool star) const noexcept
{
    if(wi.z <= 0 || wo.z <= 0)
        return 0;
    return md_->SampleWiPDF(wi.Normalize(), wo.Normalize());
}

} // namespace Atrc
