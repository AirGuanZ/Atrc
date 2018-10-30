#include <Atrc/Material/Fresnel.h>
#include <Atrc/Utility/ParamParser.h>

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

float ComputeSchlickApproximation(float etaI, float etaT, float cosThetaI)
{
    if(cosThetaI < 0.0)
    {
        std::swap(etaI, etaT);
        cosThetaI = -cosThetaI;
    }

    float R0 = (etaI - etaT) / (etaI + etaT);
    float t = 1 - cosThetaI;
    float t2 = t * t;
    R0 *= R0;
    return R0 + (1 - R0) * (t2 * t2 * t);
}

FresnelConductor::FresnelConductor(const Spectrum &etaI, const Spectrum &etaT, const Spectrum &k)
    : etaI(etaI), etaT(etaT), k(k)
{

}

Spectrum FresnelConductor::Eval(float cosThetaI) const
{
    return ComputeFresnelConductor(etaI, etaT, k, cosThetaI);
}

FresnelDielectric::FresnelDielectric(float etaI, float etaT)
    : Dielectric(etaI, etaT)
{
    
}

Spectrum FresnelDielectric::Eval(float cosThetaI) const
{
    return Spectrum(ComputeFresnelDielectric(etaI, etaT, cosThetaI));
}

SchlickApproximation::SchlickApproximation(float etaI, float etaT)
    : Dielectric(etaI, etaT)
{
    
}

Spectrum SchlickApproximation::Eval(float cosThetaI) const
{
    return Spectrum(ComputeSchlickApproximation(etaI, etaT, cosThetaI));
}

FresnelConductor *CreateFresnelConductor(const SceneParamGroup &params, AGZ::ObjArena<> &arena)
{
    Spectrum etaI = params.GetSpectrum("etaI");
    Spectrum etaT = params.GetSpectrum("etaT");
    Spectrum k    = params.GetSpectrum("k");
    return arena.Create<FresnelConductor>(etaI, etaT, k);
}

FresnelDielectric *CreateFresnelDielectric(const SceneParamGroup &params, AGZ::ObjArena<> &arena)
{
    float etaI = params.GetFloat("etaI");
    float etaT = params.GetFloat("etaT");
    return arena.Create<FresnelDielectric>(etaI, etaT);
}

SchlickApproximation *CreateSchlickApproximation(const SceneParamGroup &params, AGZ::ObjArena<> &arena)
{
    float etaI = params.GetFloat("etaI");
    float etaT = params.GetFloat("etaT");
    return arena.Create<SchlickApproximation>(etaI, etaT);
}

Fresnel *CreateFresnel(const SceneParamGroup &params, AGZ::ObjArena<> &arena)
{
    auto &s = params.GetValue("Fresnel");
    if(s == "FresnelConductor")
        return CreateFresnelConductor(params.GetGroup(s), arena);
    if(s == "FresnelDielectric")
        return CreateFresnelDielectric(params.GetGroup(s), arena);
    if(s == "SchlickApproximation")
        return CreateSchlickApproximation(params.GetGroup(s), arena);
    throw ArgumentException(("Unknown Fresnel type: " + s).ToStdString());
}

Dielectric *CreateDielectric(const SceneParamGroup &params, AGZ::ObjArena<> &arena)
{
    auto &s = params.GetValue("Dielectric");
    if(s == "FresnelDielectric")
        return CreateFresnelDielectric(params.GetGroup(s), arena);
    if(s == "SchlickApproximation")
        return CreateSchlickApproximation(params.GetGroup(s), arena);
    throw ArgumentException(("Unknown Dielectric type: " + s).ToStdString());
}

AGZ_NS_END(Atrc)
