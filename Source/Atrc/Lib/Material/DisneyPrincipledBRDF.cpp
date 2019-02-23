#include <Atrc/Lib/Material/DisneyPrincipledBRDF.h>
#include <Atrc/Lib/Material/Utility/MaterialHelper.h>

namespace Atrc
{
    
namespace
{
    
class DisneyBRDF : public BSDF
{
    const Spectrum baseColor_;
    const Real subsurface_;
    const Real metallic_;
    const Real specular_;
    const Real specularTint_;
    const Real roughness_;
    const Real anisotropic_;
    const Real sheen_;
    const Real sheenTint_;
    const Real clearcoat_;
    const Real clearcoatGloss_;

    static constexpr BSDFType TYPE = BSDFType(BSDF_REFLECTION | BSDF_NONESPECULAR);

public:

    DisneyBRDF(
        const Spectrum &baseColor,
        Real subsurface,
        Real metallic,
        Real specular,
        Real specularTint,
        Real roughness,
        Real anisotropic,
        Real sheen,
        Real sheenTint,
        Real clearcoat,
        Real clearcoatGloss) noexcept
        : baseColor_(baseColor),
          subsurface_(subsurface),
          metallic_(metallic),
          specular_(specular),
          specularTint_(specularTint),
          roughness_(roughness),
          anisotropic_(anisotropic),
          sheen_(sheen),
          sheenTint_(sheenTint),
          clearcoat_(clearcoat),
          clearcoatGloss_(clearcoatGloss)
    {
        
    }

    Spectrum GetAlbedo(BSDFType type) const noexcept override
    {
        if(Contains(type, TYPE))
            return baseColor_;
        return Spectrum();
    }

    Spectrum Eval(
        const CoordSystem &shd, const CoordSystem &geo,
        const Vec3 &wi, const Vec3 &wo, BSDFType type, bool star) const noexcept override;

    std::optional<SampleWiResult> SampleWi(
        const CoordSystem &shd, const CoordSystem &geo,
        const Vec3 &wo, BSDFType type, bool star, const Vec3 &sample) const noexcept override;

    Real SampleWiPDF(
        const CoordSystem &shd, const CoordSystem &geo,
        const Vec3 &wi, const Vec3 &wo, BSDFType type, bool star) const noexcept override;
};

} // namespace null

DisneyBRDFMaterial::DisneyBRDFMaterial(
    const Texture *baseColor,
    const Texture *subsurface,
    const Texture *metallic,
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
      subsurface_(subsurface),
      metallic_(metallic),
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

ShadingPoint DisneyBRDFMaterial::GetShadingPoint(const Intersection &inct, Arena &arena) const
{
    ShadingPoint ret;
    ret.uv = inct.usr.uv;
    ret.coordSys = MatHelper::ComputeShadingCoordSystem(inct, normalMapper_);

    Spectrum baseColor  = baseColor_     ->Sample(ret.uv);
    Real subsurface     = subsurface_    ->Sample1(ret.uv);
    Real metallic       = metallic_      ->Sample1(ret.uv);
    Real specular       = specular_      ->Sample1(ret.uv);
    Real specularTint   = specularTint_  ->Sample1(ret.uv);
    Real roughness      = roughness_     ->Sample1(ret.uv);
    Real anisotropic    = anisotropic_   ->Sample1(ret.uv);
    Real sheen          = sheen_         ->Sample1(ret.uv);
    Real sheenTint      = sheenTint_     ->Sample1(ret.uv);
    Real clearcoat      = clearcoat_     ->Sample1(ret.uv);
    Real clearcoatGloss = clearcoatGloss_->Sample1(ret.uv);

    auto bsdf = arena.Create<DisneyBRDF>(
        baseColor, subsurface, metallic, specular, specularTint,
        roughness, anisotropic, sheen, sheenTint, clearcoat, clearcoatGloss);
    ret.bsdf = bsdf;

    return ret;
}

} // namespace Atrc
