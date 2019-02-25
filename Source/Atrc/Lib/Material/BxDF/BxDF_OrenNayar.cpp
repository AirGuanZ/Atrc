#include <Atrc/Lib/Material/BxDF/BxDF_OrenNayar.h>

namespace Atrc
{
    
BxDF_OrenNayar::BxDF_OrenNayar(const Spectrum &albedo, Real sigma) noexcept
    : BxDF(BSDFType(BSDF_REFLECTION | BSDF_NONSPECULAR)), albedo_(albedo)
{
    Real sigma2 = sigma * sigma;
    A_ = 1 - sigma2 / (2 * sigma2 + 2 * Real(0.33));
    B_ = Real(0.45) * sigma2 / (sigma2 + Real(0.09));
}

Spectrum BxDF_OrenNayar::GetAlbedo() const noexcept
{
    return albedo_;
}

Spectrum BxDF_OrenNayar::Eval(const Vec3 &wi, const Vec3 &wo, [[maybe_unused]] bool star) const noexcept
{
    if(wi.z <= 0 || wo.z <= 0)
        return Spectrum();

    Vec3 lwi = wi.Normalize(), lwo = wo.Normalize();
    Real phiI = Phi(lwi), phiO = Phi(lwo);
    auto [beta, alpha] = std::minmax(Arccos(CosTheta(lwi)), Arccos(CosTheta(lwo)));

    return albedo_ / PI * (A_ + B_ * Max(Real(0), Cos(phiI - phiO)) * Sin(alpha) * Tan(beta));
}

} // namespace Atrc
