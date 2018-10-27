#include <Atrc/Material/BxDF/PerfectSpecular.h>

AGZ_NS_BEG(Atrc)

namespace
{
    // 计算折射方向，发生全反射时返回None
    Option<Vec3> GetRefractDirection(const Vec3 &wo, const Vec3 &nor, float eta)
    {
        Real cosThetaI = Abs(wo.z);
        Real sinThetaI2 = Max(0.0, 1 - cosThetaI * cosThetaI);
        Real sinThetaT2 = eta * eta * sinThetaI2;
        if(sinThetaT2 >= 1.0)
            return None;
        Real cosThetaT = Sqrt(1 - sinThetaT2);
        return (eta * cosThetaI - cosThetaT) * nor - eta * wo;
    }
}

PerfectSpecular::PerfectSpecular(const Spectrum &rc, const Dielectric *fresnel)
    : BxDF(BXDF_SPECULAR | BXDF_TRANSMISSION | BXDF_REFLECTION), rc_(rc), fresnel_(fresnel)
{
    AGZ_ASSERT(fresnel_);
}

Spectrum PerfectSpecular::Eval(const Vec3 &wi, const Vec3 &wo) const
{
    return Spectrum();
}

Option<BxDFSampleWiResult> PerfectSpecular::SampleWi(const Vec3 &wo) const
{
    Vec3 nor = wo.z > 0.0 ? Vec3::UNIT_Z() : -Vec3::UNIT_Z();

    // 先算fresnel量，根据概率选反射还是折射
    auto Fr = fresnel_->Eval(float(wo.z));
    if(Rand() < Fr.r)
    {
        BxDFSampleWiResult ret;
        ret.wi   = /*2 * Abs(wo.z) * nor - wo*/Vec3(-wo.xy(), wo.z);
        ret.coef = rc_ * Fr / Abs(float(ret.wi.z));
        ret.pdf  = Fr.r;
        ret.type = BxDFType(BXDF_SPECULAR | BXDF_REFLECTION);
        return ret;
    }

    // 折射对应的时透明物体，故wo可能是从内部往外部，应区别对待

    float etaI = wo.z > 0.0 ? fresnel_->GetEtaI() : fresnel_->GetEtaT();
    float etaT = wo.z > 0.0 ? fresnel_->GetEtaT() : fresnel_->GetEtaI();

    // 折射方向

    float eta = etaI / etaT;
    auto wi = GetRefractDirection(wo, nor, eta);

    BxDFSampleWiResult ret;
    ret.wi   = wi->Normalize();
    ret.pdf  = 1.0 - Fr.r;
    ret.coef = eta * eta * rc_ * (Spectrum(1.0f) - Fr)
             / Abs(ret.wi.z);
    ret.type = BxDFType(BXDF_SPECULAR | BXDF_TRANSMISSION);

    return ret;
}

Real PerfectSpecular::SampleWiPDF(const Vec3 &wi, const Vec3 &wo) const
{
    return 0.0;
}

AGZ_NS_END(Atrc)
