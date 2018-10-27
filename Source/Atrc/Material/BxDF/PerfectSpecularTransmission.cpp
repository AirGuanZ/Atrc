#include <Atrc/Material/BxDF/PerfectSpecularTransmission.h>

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

PerfectSpecularTransmission::PerfectSpecularTransmission(const Spectrum &rc, const Dielectric *fresnel)
    : BxDF(BXDF_SPECULAR | BXDF_TRANSMISSION), rc_(rc), fresnel_(fresnel)
{
    AGZ_ASSERT(fresnel_);
}

Spectrum PerfectSpecularTransmission::Eval(const Vec3 &wi, const Vec3 &wo) const
{
    return Spectrum();
}

Option<BxDFSampleWiResult> PerfectSpecularTransmission::SampleWi(const Vec3 &wo) const
{
    // 折射对应的时透明物体，故wo可能是从内部往外部，应区别对待

    float etaI = wo.z > 0.0 ? fresnel_->GetEtaI() : fresnel_->GetEtaT();
    float etaT = wo.z > 0.0 ? fresnel_->GetEtaT() : fresnel_->GetEtaI();
    Vec3 nor = wo.z > 0.0 ? Vec3::UNIT_Z() : -Vec3::UNIT_Z();

    // 折射方向

    float eta = etaI / etaT;
    auto wi = GetRefractDirection(wo, nor, eta);
    if(!wi)
        return None;

    auto Fr = fresnel_->Eval(float(wi->z));

    BxDFSampleWiResult ret;
    ret.wi   = wi->Normalize();
    ret.pdf  = 1.0;
    ret.coef = eta * eta * rc_ * (Spectrum(1.0f) - Fr) / Abs(ret.wi.z);
    ret.type = type_;

    return ret;
}

Real PerfectSpecularTransmission::SampleWiPDF(const Vec3 &wi, const Vec3 &wo) const
{
    return 0.0;
}

AGZ_NS_END(Atrc)
