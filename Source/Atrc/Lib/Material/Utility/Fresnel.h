#pragma once

#include <Atrc/Lib/Core/Common.h>

namespace Atrc
{
    
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
    Spectrum etaI, etaT, k;
    Spectrum eta2, etaK2;

public:

    FresnelConductor(const Spectrum &etaI, const Spectrum &etaT, const Spectrum &k) noexcept;

    Spectrum Eval(Real cosThetaI) const noexcept override;
};

// 绝缘体的fresnel项
class Dielectric : public Fresnel
{
protected:

    Real etaI, etaT;

public:

    Dielectric(Real etaI, Real etaT) noexcept;
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
