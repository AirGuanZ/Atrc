#include <Atrc/Lib/Material/BxDF/BxDF_DisneyDiffuse.h>
#include <Atrc/Lib/Material/DisneyDiffuse.h>
#include <Atrc/Lib/Material/Utility/BxDF2BSDF.h>
#include <Atrc/Lib/Material/Utility/MaterialHelper.h>

namespace Atrc
{
    
DisneyDiffuseMaterial::DisneyDiffuseMaterial(const Texture *baseColor, const Texture *subsurface, const Texture *roughness, const NormalMapper *normalMapper) noexcept
    : baseColor_(baseColor), subsurface_(subsurface), roughness_(roughness), normalMapper_(normalMapper)
{
    
}

ShadingPoint DisneyDiffuseMaterial::GetShadingPoint(const Intersection &inct, Arena &arena) const
{
    ShadingPoint ret;
    ret.uv = inct.usr.uv;
    ret.coordSys = MatHelper::ComputeShadingCoordSystem(inct, normalMapper_);

    Spectrum baseColor = baseColor_->Sample(ret.uv);
    Real subsurface = subsurface_->Sample1(ret.uv);
    Real roughness = roughness_->Sample1(ret.uv);
    //BxDF *bxdf = arena.Create<BxDF_DisneyDiffuse>(baseColor, subsurface, roughness);
    //
    //auto bsdf = arena.Create<BxDFAggregate<1>>();
    //bsdf->AddBxDF(bxdf);
    //ret.bsdf = bsdf;
    ret.bsdf = arena.Create<BxDF2BSDF<BxDF_DisneyDiffuse>>(baseColor, subsurface, roughness);

    return ret;
}

} // namespace Atrc
