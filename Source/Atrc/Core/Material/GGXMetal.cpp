#include <Atrc/Core/Material/BxDF/BxDF_SpecularReflection.h>
#include <Atrc/Core/Material/BxDF/BxDF_Microfacet.h>
#include <Atrc/Core/Material/GGXMetal.h>
#include <Atrc/Core/Material/Utility/BxDF2BSDF.h>
#include <Atrc/Core/Material/Utility/Fresnel.h>
#include <Atrc/Core/Material/Utility/MaterialHelper.h>

namespace Atrc
{

GGXMetal::GGXMetal(
    const Fresnel *fresnel,
    const Texture *rc, const Texture *roughness, const NormalMapper *normalMapper) noexcept
    : fresnel_(fresnel), rc_(rc), roughness_(roughness), normalMapper_(normalMapper)
{
    AGZ_ASSERT(fresnel && rc && roughness && normalMapper);
}

ShadingPoint GGXMetal::GetShadingPoint(const Intersection &inct, Arena &arena) const
{
    ShadingPoint ret;
    ret.uv = inct.usr.uv;
    ret.coordSys = MatHelper::ComputeShadingCoordSystem(inct, normalMapper_);

    Spectrum rc = rc_->Sample(ret.uv);
    Real roughness = roughness_->Sample1(ret.uv);

    //auto bsdf = arena.Create<BxDFAggregate<1>>(ret.coordSys, inct.coordSys);
    //
    //if(roughness < Real(0.04))
    //{
    //    auto ms = arena.Create<BxDF_SpecularReflection>(fresnel_, rc);
    //    bsdf->AddBxDF(ms);
    //}
    //else
    //{
    //    auto mr = arena.Create<BxDF_MicrofacetReflection>(rc, roughness, fresnel_);
    //    bsdf->AddBxDF(mr);
    //}
    //
    //ret.bsdf = bsdf;

    if(roughness < Real(0.04))
        ret.bsdf = arena.Create<BxDF2BSDF<BxDF_SpecularReflection>>(fresnel_, rc);
    else
        ret.bsdf = arena.Create<BxDF2BSDF<BxDF_MicrofacetReflection>>(rc, roughness, fresnel_);

    return ret;
}

} // namespace Atrc
