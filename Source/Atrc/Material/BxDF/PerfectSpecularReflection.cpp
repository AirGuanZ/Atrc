#include <Atrc/Material/BxDF/PerfectSpecularReflection.h>

AGZ_NS_BEG(Atrc)

PerfectSpecularReflection::PerfectSpecularReflection(const Spectrum &rc, RC<const Fresnel> fresnel)
    : BxDF(BXDF_SPECULAR | BXDF_REFLECTION), rc_(rc), fresnel_(std::move(fresnel))
{
    AGZ_ASSERT(fresnel_);
}

Spectrum PerfectSpecularReflection::Eval(const Vec3 &wi, const Vec3 &wo) const
{
    return Spectrum();
}

Option<BxDFSampleWiResult> PerfectSpecularReflection::SampleWi(const Vec3 &wo) const
{
    if(wo.z <= 0.0)
        return None;

    AGZ_ASSERT(IsNormalized(wo));

    if(wo.z <= 0.0)
        return None;

    BxDFSampleWiResult ret;
    ret.wi   = 2 * wo.z * Vec3::UNIT_Z() - wo;
    ret.coef = rc_ * fresnel_->Eval(float(wo.z)) / float(ret.wi.z);
    ret.pdf  = 1.0;
    ret.type = type_;

    return ret;
}

Real PerfectSpecularReflection::SampleWiPDF(const Vec3 &wi, const Vec3 &wo) const
{
    return 0.0;
}

AGZ_NS_END(Atrc)
