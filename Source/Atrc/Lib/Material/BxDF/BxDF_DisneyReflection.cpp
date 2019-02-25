#include <Atrc/Lib/Material/BxDF/BxDF_DisneyReflection.h>

namespace Atrc
{
    
BxDF_DisneyReflection::BxDF_DisneyReflection(
    const Spectrum &baseColor,
    Real metallic,
    Real subsurface,
    Real specular,
    Real specularTint,
    Real roughness,
    Real anisotropic,
    Real sheen,
    Real sheenTint,
    Real clearcoat,
    Real clearcoatGloss) noexcept
    : BxDF(BSDFType(BSDF_REFLECTION | BSDF_NONSPECULAR)),
      baseColor_(baseColor),
      metallic_(Saturate(metallic)),
      subsurface_(Saturate(subsurface)),
      specular_(Saturate(specular)),
      specularTint_(Saturate(specularTint)),
      roughness_(Clamp(roughness, Real(0.05), Real(1))),
      anisotropic_(Saturate(anisotropic)),
      sheen_(Saturate(sheen)),
      sheenTint_(Saturate(sheenTint)),
      clearcoat_(Saturate(clearcoat)),
      clearcoatGloss_(Saturate(clearcoatGloss))
{
    Real aspect = Sqrt(1 - Real(0.9) * anisotropic);
    ax_ = Sqr(roughness_) / aspect;
    ay_ = Sqr(roughness_) * aspect;
}

Spectrum BxDF_DisneyReflection::Eval(const Vec3 &wi, const Vec3 &wo, bool star) const noexcept
{
    // TODO
    return Spectrum();
}

std::optional<BxDF::SampleWiResult> BxDF_DisneyReflection::SampleWi(const Vec3& wo, bool star, const Vec3& sample) const noexcept
{
    return std::nullopt;
}

Real BxDF_DisneyReflection::SampleWiPDF(const Vec3& wi, const Vec3& wo, bool star) const noexcept
{
    return 0;
}

} // namespace Atrc
