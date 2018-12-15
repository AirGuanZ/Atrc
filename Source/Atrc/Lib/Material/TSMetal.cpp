#include <Atrc/Lib/Material/MicrofacetDistribution/BlinnPhong.h>
#include <Atrc/Lib/Material/TSMetal.h>
#include <Atrc/Lib/Material/Utility/BxDFAggregate.h>
#include <Atrc/Lib/Material/Utility/Fresnel.h>
#include "BxDF/BxDF_TorranceSparrow.h"

namespace Atrc
{

TSMetal::TSMetal(
    const Spectrum &etaI, const Spectrum &etaT, const Spectrum &k,
    const Texture *rc, const Texture *roughness, const NormalMapper *normalMapper) noexcept
    : etaI_(etaI), etaT_(etaT), k_(k), rc_(rc), roughness_(roughness), normalMapper_(normalMapper)
{
    AGZ_ASSERT(rc && roughness && normalMapper);
}

ShadingPoint TSMetal::GetShadingPoint(const Intersection &inct, Arena &arena) const
{
    ShadingPoint ret;
    ret.uv = inct.usr.uv;
    ret.coordSys = inct.usr.coordSys;

    Vec3 localNormal = normalMapper_->GetLocalNormal(ret.uv);
    Vec3 worldNormal = ret.coordSys.Local2World(localNormal);
    ret.coordSys = CoordSystem::FromEz(worldNormal);

    Spectrum rc = rc_->Sample(ret.uv);
    Real roughness = roughness_->Sample1(ret.uv);

    auto fresnel = arena.Create<FresnelConductor>(etaI_, etaT_, k_);
    auto md      = arena.Create<BlinnPhong>(1 / roughness);
    auto bsdf    = arena.Create<BxDFAggregate<1>>(ret.coordSys, inct.coordSys);
    auto ts      = arena.Create<BxDF_TorranceSparrow>(rc, md, fresnel);
    bsdf->AddBxDF(ts);
    ret.bsdf = bsdf;

    return ret;
}

} // namespace Atrc
