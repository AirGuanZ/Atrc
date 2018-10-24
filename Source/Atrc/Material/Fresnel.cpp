#include <Atrc/Material/Fresnel.h>

AGZ_NS_BEG(Atrc)

float ComputeFresnelDielectric(float etaI, float etaT, float cosThetaI)
{
    if(cosThetaI < 0.0)
    {
        std::swap(etaI, etaT);
        cosThetaI = -cosThetaI;
    }

    // 求解Snell方程
    float sinThetaI = Sqrt(Max(0.0f, 1 - cosThetaI * cosThetaI));
    float sinThetaT = etaI / etaT * sinThetaI;

    // 全反射
    if(sinThetaT >= 1.0f)
        return 1.0f;

    float cosThetaT = Sqrt(Max(0.0f, 1 - sinThetaT * sinThetaT));
    float para = (etaT * cosThetaI - etaI * cosThetaT) / (etaT * cosThetaI + etaI * cosThetaT);
    float perp = (etaI * cosThetaI - etaT * cosThetaT) / (etaI * cosThetaI + etaT * cosThetaT);

    return 0.5f * (para * para + perp * perp);
}

Spectrum ComputeFresnelConductor(const Spectrum &etaI, const Spectrum &etaT, const Spectrum &k, float cosThetaI)
{
    Spectrum eta = etaT / etaI;
    Spectrum etaK = k / etaI;

    float cosThetaI2 = cosThetaI * cosThetaI;
    float sinThetaI2 = Max(0.0f, 1 - cosThetaI2);
    Spectrum eta2 = eta * eta;
    Spectrum etaK2 = etaK * etaK;

    Spectrum t0 = eta2 - etaK2 - sinThetaI2;
    Spectrum a2b2 = Sqrt(t0 * t0 + 4.0f * eta2 * etaK2);
    Spectrum t1 = a2b2 + cosThetaI2;
    Spectrum a = Sqrt(0.5f * (a2b2 + t0));
    Spectrum t2 = 2.0f * cosThetaI * a;
    Spectrum rs = (t1 - t2) / (t1 + t2);

    Spectrum t3 = cosThetaI2 * a2b2 + sinThetaI2 * sinThetaI2;
    Spectrum t4 = t2 * sinThetaI2;
    Spectrum rp = rs * (t3 - t4) / (t3 + t4);

    return 0.5f * (rp + rs);
}

FresnelDielectric::FresnelDielectric(float etaI, float etaT)
    : etaI(etaI), etaT(etaT)
{
    
}

Spectrum FresnelDielectric::Eval(float cosThetaI) const
{
    return Spectrum(ComputeFresnelDielectric(etaI, etaT, cosThetaI));
}

FresnelConductor::FresnelConductor(const Spectrum &etaI, const Spectrum &etaT, const Spectrum &k)
    : etaI(etaI), etaT(etaT), k(k)
{
    
}

Spectrum FresnelConductor::Eval(float cosThetaI) const
{
    return ComputeFresnelConductor(etaI, etaT, k, cosThetaI);
}

AGZ_NS_END(Atrc)
