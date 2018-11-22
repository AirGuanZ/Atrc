#include <Atrc/Material/BxDF/PerfectSpecularReflection.h>

AGZ_NS_BEG(Atrc)

PerfectSpecularReflection::PerfectSpecularReflection(const Spectrum &rc, const Fresnel *fresnel)
    : BxDF(BXDF_SPECULAR | BXDF_REFLECTION), rc_(rc), fresnel_(fresnel)
{
    AGZ_ASSERT(fresnel_);
}

Spectrum PerfectSpecularReflection::Eval([[maybe_unused]] const Vec3 &wi, [[maybe_unused]] const Vec3 &wo) const
{
    return Spectrum();
}

Option<BxDFSampleWiResult> PerfectSpecularReflection::SampleWi(const Vec3 &wo) const
{
    AGZ_ASSERT(IsNormalized(wo));

    Vec3 nor = wo.z > 0.0 ? Vec3::UNIT_Z() : -Vec3::UNIT_Z();

    BxDFSampleWiResult ret;
    ret.wi   = 2 * Abs(wo.z) * nor - wo;
    ret.coef = rc_ * fresnel_->Eval(float(ret.wi.z)) / Abs(float(ret.wi.z));
    ret.pdf  = 1.0;
    ret.type = type_;

    return ret;
}

Real PerfectSpecularReflection::SampleWiPDF([[maybe_unused]] const Vec3 &wi, [[maybe_unused]] const Vec3 &wo) const
{
    return 0.0;
}

AGZ_NS_END(Atrc)
