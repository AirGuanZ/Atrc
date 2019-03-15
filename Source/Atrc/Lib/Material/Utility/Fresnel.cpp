#include <Atrc/Lib/Material/Utility/Fresnel.h>

namespace Atrc
{

Real ComputeFresnelDielectric(Real etaOut, Real etaIn, Real cosThetaI) noexcept
{
    if(cosThetaI < 0)
    {
        std::swap(etaOut, etaIn);
        cosThetaI = -cosThetaI;
    }

    Real sinThetaI = Sqrt(Max(Real(0), 1 - cosThetaI * cosThetaI));
    Real sinThetaT = etaOut / etaIn * sinThetaI;

    if(sinThetaT >= 1)
        return 1;

    Real cosThetaT = Sqrt(Max(Real(0), 1 - sinThetaT * sinThetaT));
    Real para = (etaIn * cosThetaI - etaOut * cosThetaT) / (etaIn * cosThetaI + etaOut * cosThetaT);
    Real perp = (etaOut * cosThetaI - etaIn * cosThetaT) / (etaOut * cosThetaI + etaIn * cosThetaT);

    return Real(0.5) * (para * para + perp * perp);
}
    
FresnelConductor::FresnelConductor(const Spectrum &etaOut, const Spectrum &etaIn, const Spectrum &k) noexcept
    : etaOut(etaOut), etaIn(etaIn), k(k)
{
    eta2 = etaIn / etaOut; eta2 *= eta2;
    etaK2 = k / etaOut; etaK2 *= etaK2;
}

Spectrum FresnelConductor::Eval(Real cosThetaI) const noexcept
{
    Real cos2 = cosThetaI * cosThetaI;
    Real sin2 = Max(Real(0), 1 - cos2);

    Spectrum t0   = eta2 - etaK2 - sin2;
    Spectrum a2b2 = Sqrt(t0 * t0 + 4 * eta2 * etaK2);
    Spectrum t1   = a2b2 + cos2;
    Spectrum a    = Sqrt(Real(0.5) * (a2b2 + t0));
    Spectrum t2   = 2 * cosThetaI * a;
    Spectrum rs   = (t1 - t2) / (t1 + t2);

    Spectrum t3 = cos2 * a2b2 + sin2 * sin2;
    Spectrum t4 = t2 * sin2;
    Spectrum rp = rs * (t3 - t4) / (t3 + t4);

    return Real(0.5) * (rp + rs);
}

Dielectric::Dielectric(Real etaOut, Real etaIn) noexcept
    : etaOut(etaOut), etaIn(etaIn)
{

}

namespace
{
    Real ComputeSchlickApproximation(Real etaI, Real etaT, Real cosThetaI)
    {
        if(cosThetaI < 0)
        {
            std::swap(etaI, etaT);
            cosThetaI = -cosThetaI;
        }

        Real sinThetaI = Sqrt(Max(Real(0), 1 - cosThetaI * cosThetaI));
        Real sinThetaT = etaI / etaT * sinThetaI;

        if(sinThetaT >= 1)
            return 1;

        Real R0 = (etaI - etaT) / (etaI + etaT);
        R0 *= R0;
        Real t = 1 - cosThetaI;
        Real t2 = t * t;

        return R0 + (1 - R0) * (t2 * t2 * t);
    }
} // namespace null

Spectrum FresnelDielectric::Eval(Real cosThetaI) const noexcept
{
    return Spectrum(ComputeFresnelDielectric(etaOut, etaIn, cosThetaI));
}

Spectrum SchlickApproximation::Eval(Real cosThetaI) const noexcept
{
    return Spectrum(ComputeSchlickApproximation(etaOut, etaIn, cosThetaI));
}

} // namespace Atrc
