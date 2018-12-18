#include <Atrc/Lib/Material/BxDF/BxDF_TorranceSparrow.h>

namespace Atrc
{
    
BxDF_TorranceSparrow::BxDF_TorranceSparrow(const Spectrum &rc, const MicrofacetDistribution *md, const Fresnel *fresnel) noexcept
    : BxDF(BSDFType(BSDF_REFLECTION | BSDF_GLOSSY)), rc_(rc), md_(md), fresnel_(fresnel)
{
    AGZ_ASSERT(md && fresnel);
}

Spectrum BxDF_TorranceSparrow::GetAlbedo() const noexcept
{
    return rc_;
}

Spectrum BxDF_TorranceSparrow::Eval(const CoordSystem &geoInShd, const Vec3 &wi, const Vec3 &wo) const noexcept
{
    if(wi.z <= 0 || wo.z <= 0 || !geoInShd.InPositiveHemisphere(wi) || !geoInShd.InPositiveHemisphere(wo))
        return Spectrum();

    Vec3 nWi = wi.Normalize(), nWo = wo.Normalize();
    Vec3 H = (nWi + nWo).Normalize();
    Real G = md_->G(H, nWi, nWo);
    if(!G)
        return Spectrum();
    Spectrum fr = fresnel_->Eval(Dot(nWi, H));
    return rc_ * fr * md_->D(H) * G / (4 * nWi.z * nWo.z);
}

Option<BxDF::SampleWiResult> BxDF_TorranceSparrow::SampleWi(const CoordSystem &geoInShd, const Vec3 &wo, const Vec2 &sample) const noexcept
{
    Vec3 nWo = wo.Normalize();
    auto mdSample = md_->SampleWi(geoInShd, nWo, sample);
    if(!mdSample)
        return None;

    SampleWiResult ret;
    ret.coef = Eval(geoInShd, mdSample->wi, wo);
    if(!ret.coef)
        return None;
    ret.wi      = mdSample->wi;
    ret.pdf     = mdSample->pdf;
    ret.type    = type_;
    ret.isDelta = false;

    return ret;
}

Real BxDF_TorranceSparrow::SampleWiPDF(const CoordSystem &geoInShd, const Vec3 &wi, const Vec3 &wo) const noexcept
{
    if(wi.z <= 0 || wo.z <= 0 || !geoInShd.InPositiveHemisphere(wi) || !geoInShd.InPositiveHemisphere(wo))
        return 0;
    return md_->SampleWiPDF(geoInShd, wi.Normalize(), wo.Normalize());
}

} // namespace Atrc
