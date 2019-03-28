#pragma once

#include <Atrc/Core/Core/Common.h>

namespace Atrc
{

Real ComputeFresnelDielectric(Real etaOut, Real etaIn, Real cosThetaI) noexcept;
    
// Fresnel项
class Fresnel
{
public:

    virtual ~Fresnel() = default;

    virtual Spectrum Eval(Real cosThetaI) const noexcept = 0;
};

// 导体的fresnel项
class FresnelConductor : public Fresnel
{
    Spectrum etaOut, etaIn, k;
    Spectrum eta2, etaK2;

public:

    FresnelConductor(const Spectrum &etaOut, const Spectrum &etaIn, const Spectrum &k) noexcept;

    Spectrum Eval(Real cosThetaI) const noexcept override;
};

// 绝缘体的fresnel项
class Dielectric : public Fresnel
{
protected:

    Real etaOut, etaIn;

public:

    Dielectric(Real etaOut, Real etaIn) noexcept;

    Real GetEtaOut() const noexcept { return etaOut; }
    Real GetEtaIn() const noexcept { return etaIn; }
};

// 恒为1的fresnel
class AlwaysOneFresnel : public Dielectric
{
public:

    AlwaysOneFresnel() : Dielectric(1, 1) { }

    Spectrum Eval(Real cosThetaI) const noexcept override
    {
        return Spectrum(1);
    }
};

// 使用fresnel公式的绝缘体fresnel项
class FresnelDielectric : public Dielectric
{
public:

    using Dielectric::Dielectric;

    Spectrum Eval(Real cosThetaI) const noexcept override;
};

// 使用Schlick近似公式的fresnel项
class SchlickApproximation : public Dielectric
{
public:

    using Dielectric::Dielectric;

    Spectrum Eval(Real cosThetaI) const noexcept override;
};

} // namespace Atrc
