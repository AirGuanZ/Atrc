#include <Atrc/Lib/Material/BxDF/BxDF_DisneyPrincipled.h>

namespace
{

using namespace Atrc;

template<typename T>
auto Mix(const T &left, const T &right, Real factor)
{
    return (1 - factor) * left + factor * right;
}

Real Diffuse(Real cosThetaI, Real cosThetaO, Real cosThetaD, Real roughness, Real subsurfaceWeight)
{
    Real cosThetaD2 = cosThetaD * cosThetaD;
    Real FD90 = Real(0.5) + 2 * cosThetaD2 * roughness;
    Real x = 1 - cosThetaI, x2 = x * x, x5 = x2 * x2 * x;
    Real y = 1 - cosThetaO, y2 = y * y, y5 = y2 * y2 * y;
    Real diffuse = (1 + (FD90 - 1) * x5) * (1 + (FD90 - 1) * y5);

    if(!subsurfaceWeight)
        return 1 / PI * diffuse;

    Real Fss90 = cosThetaD2 * roughness;
    Real Fss = (1 + (Fss90 - 1) * x5) * (1 + (Fss90 - 1) * y5);
    Real subsurface = Real(1.25) * (Fss * (1 / (cosThetaI + cosThetaO) - Real(0.5)) + Real(0.5));

    return 1 / PI * Mix(diffuse, subsurface, subsurfaceWeight);
}



} // namespace anonymous

namespace Atrc
{

DisneyPrincipledBxDF::DisneyPrincipledBxDF(
    const Spectrum &rc,
    Real roughness,
    Real specular,
    Real specularTint,
    Real metallic,
    Real sheen,
    Real sheenTint,
    Real subsurface,
    Real clearCoat,
    Real clearCoatGloss) noexcept
    : BxDF(BSDFType(BSDF_REFLECTION | BSDF_GLOSSY)),
      rc_            (rc),
      roughness_     (roughness),
      specular_      (specular),
      specularTint_  (specularTint),
      metallic_      (metallic),
      sheen_         (sheen),
      sheenTint_     (sheenTint),
      subsurface_    (subsurface),
      clearCoat_     (clearCoat),
      clearCoatGloss_(clearCoatGloss),
      gamma_(1.7) // IMPROVE: 暂时写成定值，理论上可取的范围是[1, 2]
{

}

} // namespace Atrc
