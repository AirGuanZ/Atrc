#include <Atrc/Core/Material/BxDF/BxDF_Diffuse.h>
#include <Atrc/Core/Material/BxDF/BxDF_OrenNayar.h>
#include <Atrc/Core/Material/Utility/BxDF2BSDF.h>
#include <Atrc/Core/Material/Utility/MaterialHelper.h>
#include <Atrc/Core/Material/ONMatte.h>

namespace Atrc
{
    
ONMatte::ONMatte(const Texture *albedoMap, const Texture *sigmaMap, const NormalMapper *normalMapper) noexcept
    : albedoMap_(albedoMap), sigmaMap_(sigmaMap), normalMapper_(normalMapper)
{
    AGZ_ASSERT(albedoMap && sigmaMap && normalMapper);
}

ShadingPoint ONMatte::GetShadingPoint(const Intersection &inct, Arena &arena) const
{
    ShadingPoint ret;
    ret.uv = inct.usr.uv;
    ret.coordSys = MatHelper::ComputeShadingCoordSystem(inct, normalMapper_);

    /*ret.coordSys = inct.usr.coordSys;
    Vec3 localNormal = normalMapper_->GetLocalNormal(ret.uv);
    Vec3 worldNormal = ret.coordSys.Local2World(localNormal);
    ret.coordSys = CoordSystem::FromEz(worldNormal);*/

    Spectrum albedo = albedoMap_->Sample(ret.uv);
    Real sigma      = sigmaMap_->Sample1(ret.uv);

    //auto bsdf = arena.Create<BxDFAggregate<1>>(ret.coordSys, inct.coordSys);
    //if(!sigma)
    //    bsdf->AddBxDF(arena.Create<BxDF_Diffuse>(albedo));
    //else
    //    bsdf->AddBxDF(arena.Create<BxDF_OrenNayar>(albedo, sigma));
    //ret.bsdf = bsdf;

    if(!sigma)
        ret.bsdf = arena.Create<BxDF2BSDF<BxDF_Diffuse>>(albedo);
    else
        ret.bsdf = arena.Create<BxDF2BSDF<BxDF_OrenNayar>>(albedo, sigma);

    return ret;
}

} // namespace Atrc
