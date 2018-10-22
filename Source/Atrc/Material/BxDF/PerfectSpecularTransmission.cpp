#include <Atrc/Material/BxDF/PerfectSpecularTransmission.h>

AGZ_NS_BEG(Atrc)

PerfectSpecularTransmission::PerfectSpecularTransmission(const Spectrum &rc, RC<Fresnel> fresnel)
    : BxDF(BXDF_SPECULAR | BXDF_TRANSMISSION), rc_(rc), fresnel_(std::move(fresnel))
{
    AGZ_ASSERT(fresnel_);
}

Spectrum PerfectSpecularTransmission::Eval(const Vec3 &wi, const Vec3 &wo) const
{
    return Spectrum();
}

Option<BxDFSampleWiResult> PerfectSpecularTransmission::SampleWi(const Vec3 &wo) const
{
    // TODO
    return None;
}

Real PerfectSpecularTransmission::SampleWiPDF(const Vec3 &wi, const Vec3 &wo) const
{
    return 0.0;
}

AGZ_NS_END(Atrc)
