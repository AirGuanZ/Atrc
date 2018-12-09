#include <Atrc/Material/BxDF/TorranceSparrow.h>

AGZ_NS_BEG(Atrc)

float TorranceSparrow::G(const Vec3 &wi, const Vec3 &wo, const Vec3 &H) const
{
    float NdH = Abs(float(CosTheta(H)));
    float NdWi = Abs(float(CosTheta(wi)));
    float NdWo = Abs(float(CosTheta(wo)));
    float WodH = Abs(float(Dot(wo, H)));
    return Min(1.0f, Min(2 * NdH * NdWo / WodH,
                         2 * NdH * NdWi / WodH));
}

TorranceSparrow::TorranceSparrow(const Spectrum &rc, const MicrofacetDistribution *md, const Fresnel *fresnel)
    : BxDF(BXDF_REFLECTION | BXDF_GLOSSY), rc_(rc), md_(md), fresnel_(fresnel)
{
    AGZ_ASSERT(md && fresnel);
}

Spectrum TorranceSparrow::Eval(const LocalCoordSystem &localShdCoord, const Vec3 &wi, const Vec3 &wo) const
{
    if(wi.z <= 0.0 || wo.z <= 0.0)
        return Spectrum();

    Vec3 H = (wi + wo).Normalize();
    Spectrum Fr = fresnel_->Eval(float(Dot(wi, H)));

    return rc_ * Fr * md_->Eval(H) * G(wi, wo, H) /
           (4 * CosTheta(wi) * CosTheta(wo));
}

Option<BxDFSampleWiResult> TorranceSparrow::SampleWi(const LocalCoordSystem &localShdCoord, const Vec3 &wo) const
{
    auto mdSam = md_->SampleWi(wo);
    if(!mdSam)
        return None;

    BxDFSampleWiResult ret;
    ret.coef = Eval(localShdCoord, mdSam->wi, wo);
    ret.wi   = mdSam->wi;
    ret.pdf  = mdSam->pdf;
    ret.type = type_;

    return ret;
}

Real TorranceSparrow::SampleWiPDF(const LocalCoordSystem &localShdCoord, const Vec3 &wi, const Vec3 &wo) const
{
    return md_->SampleWiPDF(wi, wo);
}

AGZ_NS_END(Atrc)
