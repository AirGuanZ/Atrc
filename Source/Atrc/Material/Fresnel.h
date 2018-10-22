#pragma once

#include <Atrc/Core/Core.h>

// 这部分实现参考了PBRT-3rd

AGZ_NS_BEG(Atrc)

// 绝缘体Fresnel反射比
float ComputeFresnelDielectric(float etaI, float etaT, float cosThetaT);

// 导体Fresnel反射比
Spectrum ComputeFresnelConductor(const Spectrum &etaI, const Spectrum &etaT, const Spectrum &k, float cosThetaI);

class Fresnel
{
public:

    virtual ~Fresnel() = default;

    virtual Spectrum Eval(float cosThetaI) const = 0;
};

class FresnelDielectric : public Fresnel
{
    float etaI, etaT;

public:

    FresnelDielectric(float etaI, float etaT);

    Spectrum Eval(float cosThetaI) const;
};

class FresnelConductor : public Fresnel
{
    Spectrum etaI, etaT, k;

public:

    FresnelConductor(const Spectrum &etaI, const Spectrum &etaT, const Spectrum &k);

    Spectrum Eval(float cosThetaI) const;
};

AGZ_NS_END(Atrc)
