#include <Atrc/Lib/Material/Disney.h>
#include <Atrc/Lib/Material/BxDF/BxDF_DisneyPrincipled.h>
#include <Atrc/Lib/Material/Utility/MaterialHelper.h>
#include "Utility/BxDFAggregate.h"

namespace Atrc
{
    /*
DisneyPrincipledMaterial::DisneyPrincipledMaterial(
    const Texture *rc,
    const Texture *roughness,
    const Texture *specular,
    const Texture *specularTint,
    const Texture *metallic,
    const Texture *sheen,
    const Texture *sheenTint,
    const Texture *subsurface,
    const Texture *clearCoat,
    const Texture *clearCoatGloss,
    const Fresnel *fresnel,
    const NormalMapper *normalMapper) noexcept
    : rc_            (rc),
      roughness_     (roughness),
      specular_      (specular),
      specularTint_  (specularTint),
      metallic_      (metallic),
      sheen_         (sheen),
      sheenTint_     (sheenTint),
      subsurface_    (subsurface),
      clearCoat_     (clearCoat),
      clearCoatGloss_(clearCoatGloss),
      fresnel_       (fresnel),
      normalMapper_  (normalMapper)
{
    AGZ_ASSERT(rc && roughness && specular && specularTint && metallic);
    AGZ_ASSERT(sheen && sheenTint && subsurface && clearCoat && clearCoatGloss);
    AGZ_ASSERT(fresnel && normalMapper);
}

ShadingPoint DisneyPrincipledMaterial::GetShadingPoint(const Intersection &inct, Arena &arena) const
{
    ShadingPoint ret;
    ret.uv = inct.usr.uv;
    ret.coordSys = MatHelper::ComputeShadingCoordSystem(inct, normalMapper_);

    Spectrum rc         = rc_            ->Sample (ret.uv);
    Real roughness      = roughness_     ->Sample1(ret.uv);
    Real specular       = specular_      ->Sample1(ret.uv);
    Real specularTint   = specularTint_  ->Sample1(ret.uv);
    Real metallic       = metallic_      ->Sample1(ret.uv);
    Real subsurface     = subsurface_    ->Sample1(ret.uv);
    Real sheen          = sheen_         ->Sample1(ret.uv);
    Real sheenTint      = sheenTint_     ->Sample1(ret.uv);
    Real clearCoat      = clearCoat_     ->Sample1(ret.uv);
    Real clearCoatGloss = clearCoatGloss_->Sample1(ret.uv);

    auto bsdf = arena.Create<BxDFAggregate<1>>(ret.coordSys, inct.coordSys);
    auto bxdf = arena.Create<DisneyPrincipledBxDF>(
        rc, roughness, specular, specularTint, metallic, sheen, sheenTint, subsurface, clearCoat, clearCoatGloss);
    bsdf->AddBxDF(bxdf);
    ret.bsdf = bsdf;

    return ret;
}*/

} // namespace Atrc
