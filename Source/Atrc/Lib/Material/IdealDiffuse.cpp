#include <Atrc/Lib/Core/Entity.h>
#include <Atrc/Lib/Material/BxDF/BxDF_Diffuse.h>
#include <Atrc/Lib/Material/IdealDiffuse.h>
#include <Atrc/Lib/Material/Utility/BxDFAggregate.h>

namespace Atrc
{

IdealDiffuse::IdealDiffuse(const Texture *albedoMap, const NormalMapper *normalMapper) noexcept
    : albedoMap_(albedoMap), normalMapper_(normalMapper)
{
    AGZ_ASSERT(albedoMap && normalMapper);
}

ShadingPoint IdealDiffuse::GetShadingPoint(const Intersection &inct, Arena &arena) const noexcept
{
    ShadingPoint ret;
    ret.uv       = inct.usr.uv;
    ret.coordSys = inct.usr.coordSys;

    Vec3 localNormal = normalMapper_->GetLocalNormal(ret.uv);
    Vec3 worldNormal = ret.coordSys.Local2World(localNormal);
    ret.coordSys = CoordSystem::FromEz(worldNormal);

    Spectrum albedo = albedoMap_->Sample(ret.uv);
    BxDF *bxdf = arena.Create<BxDF_Diffuse>(albedo);

    auto bsdf = arena.Create<BxDFAggregate<1>>(ret.coordSys, inct.coordSys);
    bsdf->AddBxDF(bxdf);
    ret.bsdf = bsdf;

    return ret;
}

} // namespace Atrc
