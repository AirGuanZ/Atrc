#include <Atrc/Lib/Material/DisneyReflection.h>
#include <Atrc/Lib/Material/BxDF/BxDF_DisneyReflection.h>
#include <Atrc/Lib/Material/Utility/BxDFAggregate.h>
#include <Atrc/Lib/Material/Utility/MaterialHelper.h>

namespace Atrc
{
    
DisneyReflection::DisneyReflection(
    const Texture *baseColor,
    const Texture *metallic,
    const Texture *subsurface,
    const Texture *specular,
    const Texture *specularTint,
    const Texture *roughness,
    const Texture *anisotropic,
    const Texture *sheen,
    const Texture *sheenTint,
    const Texture *clearcoat,
    const Texture *clearcoatGloss,
    const NormalMapper *normalMapper) noexcept
    : baseColor_(baseColor),
      metallic_(metallic),
      subsurface_(subsurface),
      specular_(specular),
      specularTint_(specularTint),
      roughness_(roughness),
      anisotropic_(anisotropic),
      sheen_(sheen),
      sheenTint_(sheenTint),
      clearcoat_(clearcoat),
      clearcoatGloss_(clearcoatGloss),
      normalMapper_(normalMapper)
{

}

ShadingPoint DisneyReflection::GetShadingPoint(const Intersection &inct, Arena &arena) const
{
    ShadingPoint ret;
    ret.uv = inct.usr.uv;
    ret.coordSys = MatHelper::ComputeShadingCoordSystem(inct, normalMapper_);

    Spectrum baseColor = baseColor_->Sample(ret.uv);
    Real metallic = metallic_->Sample1(ret.uv);
    Real subsurface = subsurface_->Sample1(ret.uv);
    Real specular = specular_->Sample1(ret.uv);
    Real specularTint = specularTint_->Sample1(ret.uv);
    Real roughness = roughness_->Sample1(ret.uv);
    Real anisotropic = anisotropic_->Sample1(ret.uv);
    Real sheen = sheen_->Sample1(ret.uv);
    Real sheenTint = sheenTint_->Sample1(ret.uv);
    Real clearcoat = clearcoat_->Sample1(ret.uv);
    Real clearcoatGloss = clearcoatGloss_->Sample1(ret.uv);
    BxDF *bxdf = arena.Create<BxDF_DisneyReflection>(
        baseColor, metallic, subsurface, specular, specularTint, roughness,
        anisotropic, sheen, sheenTint, clearcoat, clearcoatGloss);

    auto bsdf = arena.Create<BxDFAggregate<1>>(ret.coordSys, inct.coordSys);
    bsdf->AddBxDF(bxdf);
    ret.bsdf = bsdf;

    return ret;
}

} // namespace Atrc
