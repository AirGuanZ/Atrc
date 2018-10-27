#pragma once

#include <Atrc/Core/Core.h>

// 这部分实现参考了PBRT-3rd

AGZ_NS_BEG(Atrc)

// 绝缘体Fresnel反射比
float ComputeFresnelDielectric(float etaI, float etaT, float cosThetaT);

// 导体Fresnel反射比
Spectrum ComputeFresnelConductor(const Spectrum &etaI, const Spectrum &etaT, const Spectrum &k, float cosThetaI);

// Schlick公式
float ComputeSchlickApproximation(float etaI, float etaT, float cosThetaI);

class Fresnel
{
public:

    virtual ~Fresnel() = default;

    virtual Spectrum Eval(float cosThetaI) const = 0;
};

class FresnelConductor : public Fresnel
{
    Spectrum etaI, etaT, k;

public:

    FresnelConductor(const Spectrum &etaI, const Spectrum &etaT, const Spectrum &k);

    Spectrum Eval(float cosThetaI) const override;
};

class Dielectric : public Fresnel
{
protected:

    float etaI, etaT;

public:

    Dielectric(float etaI, float etaT) : etaI(etaI), etaT(etaT) { }

    float GetEtaI() const { return etaI; }

    float GetEtaT() const { return etaT; }
};

class FresnelDielectric : public Dielectric
{
public:

    FresnelDielectric(float etaI, float etaT);

    Spectrum Eval(float cosThetaI) const override;
};

class SchlickApproximation : public Dielectric
{
public:

    SchlickApproximation(float etaI, float etaT);

    Spectrum Eval(float cosThetaI) const override;
};

AGZ_NS_END(Atrc)
