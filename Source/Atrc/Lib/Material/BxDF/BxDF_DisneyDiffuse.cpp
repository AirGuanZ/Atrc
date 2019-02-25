#include <Atrc/Lib/Material/BxDF/BxDF_DisneyDiffuse.h>

namespace Atrc
{
    
BxDF_DisneyDiffuse::BxDF_DisneyDiffuse(const Spectrum &baseColor, Real subsurface, Real roughness) noexcept
    : BxDF(BSDFType(BSDF_REFLECTION | BSDF_NONSPECULAR)), baseColor_(baseColor), subsurface_(subsurface), roughness_(roughness)
{
    
}

Spectrum BxDF_DisneyDiffuse::GetAlbedo() const noexcept
{
    return baseColor_;
}

namespace
{
    Real Ft(Real x) noexcept
    {
        Real t = 1 - x, t2 = t * t;
        return t2 * t2 * t;
    }

    Real f_diffuse(Real FI, Real FO, Real cosThetaD2, Real roughness) noexcept
    {
        Real FD90 = Real(0.5) + 2 * cosThetaD2 * roughness;
        return (1 + (FD90 - 1) * FI) * (1 + (FD90 - 1) * FO);
    }

    Real f_subsurface(Real cosThetaI, Real cosThetaO, Real FI, Real FO, Real cosThetaD2, Real roughness) noexcept
    {
        Real Fss90 = cosThetaD2 * roughness;
        Real Fss = (1 + (Fss90 - 1) * FI) * (1 + (Fss90 - 1) * FO);
        return Real(1.25) * (Fss * (1 / (cosThetaI + cosThetaO) - Real(0.5)) + Real(0.5));
    }

} // namespace anonymous

Spectrum BxDF_DisneyDiffuse::Eval(const Vec3 &wi, const Vec3 &wo, bool star) const noexcept
{
    Vec3 wh = (wi + wo).Normalize();
    Real cosThetaI = CosTheta(wi), cosThetaO = CosTheta(wo);
    Real cosThetaD = Dot(wi, wh), cosThetaD2 = Sqr(cosThetaD);
    Real FI = Ft(cosThetaI), FO = Ft(cosThetaO), FD = Ft(cosThetaD);

    Real f_d = subsurface_ < 1 ? f_diffuse(FI, FO, cosThetaD2, roughness_) : Real(0);
    Real f_s = subsurface_ > 0 ? f_subsurface(cosThetaI, cosThetaO, FI, FO, cosThetaD2, roughness_) : Real(0);

    return baseColor_ / PI * Mix(f_d, f_s, subsurface_);
}

} // namespace Atrc
