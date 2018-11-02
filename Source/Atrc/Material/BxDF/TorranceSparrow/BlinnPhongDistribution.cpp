#include <Atrc/Material/BxDF/TorranceSparrow/BlinnPhongDistribution.h>

AGZ_NS_BEG(Atrc)

BlinnPhongDistribution::BlinnPhongDistribution(Real e)
    : e_(e)
{
    
}

float BlinnPhongDistribution::Eval(const Vec3 &H) const
{
    return (float(e_) + 2) * (1 / (2 * float(PI))) * Pow(Abs(float(CosTheta(H))), float(e_));
}

Option<MDSampleResult> BlinnPhongDistribution::SampleWi(const Vec3 &wo) const
{
    if(wo.z <= 0)
        return None;

    Real u1 = Rand(), u2 = Rand();
    Real cosTheta = Pow(u1, 1 / (e_ + 1));
    Real sinTheta = Sqrt(Max(Real(0), 1 - cosTheta * cosTheta));
    Real phi = 2 * PI * u2;

    Vec3 H = { sinTheta * Cos(phi), sinTheta * Sin(phi), cosTheta };
    if(H.z <= 0)
        return None;

    MDSampleResult ret;
    ret.wi = 2 * Dot(wo, H) * H - wo;
    if(ret.wi.z <= 0)
        return None;
    AGZ_ASSERT(IsNormalized(ret.wi));

    ret.pdf = (e_ + 1) * Pow(cosTheta, e_) / (2 * PI * 4 * Dot(wo, H));

    return ret;
}

Real BlinnPhongDistribution::SampleWiPDF(const Vec3 &wi, const Vec3 &wo) const
{
    if(wi.z <= 0.0 || wo.z <= 0.0)
        return 0.0;

    Vec3 H = (wi + wo).Normalize();
    Real cosTheta = CosTheta(H);

    return (e_ + 1) * Pow(cosTheta, e_) / (2 * PI * 4 * Dot(wo, H));
}

AGZ_NS_END(Atrc)
