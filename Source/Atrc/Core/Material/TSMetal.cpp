#include <Atrc/Core/Material/BxDF/BxDF_SpecularReflection.h>
#include <Atrc/Core/Material/BxDF/BxDF_TorranceSparrow.h>
#include <Atrc/Core/Material/MicrofacetDistribution/BlinnPhong.h>
#include <Atrc/Core/Material/TSMetal.h>
#include <Atrc/Core/Material/Utility/BxDF2BSDF.h>
#include <Atrc/Core/Material/Utility/Fresnel.h>
#include <Atrc/Core/Material/Utility/MaterialHelper.h>

namespace Atrc
{

TSMetal::TSMetal(
    const Fresnel *fresnel,
    const Texture *rc, const Texture *roughness, const NormalMapper *normalMapper) noexcept
    : fresnel_(fresnel), rc_(rc), roughness_(roughness), normalMapper_(normalMapper)
{
    AGZ_ASSERT(fresnel && rc && roughness && normalMapper);
}

ShadingPoint TSMetal::GetShadingPoint(const Intersection &inct, Arena &arena) const
{
    ShadingPoint ret;
    ret.uv = inct.usr.uv;
    ret.coordSys = MatHelper::ComputeShadingCoordSystem(inct, normalMapper_);

    Spectrum rc = rc_->Sample(ret.uv);
    Real roughness = roughness_->Sample1(ret.uv);

    //auto bsdf = arena.Create<BxDFAggregate<1>>(ret.coordSys, inct.coordSys);
    //
    //if(!roughness)
    //{
    //    auto ms = arena.Create<BxDF_SpecularReflection>(fresnel_, rc);
    //    bsdf->AddBxDF(ms);
    //}
    //else
    //{
    //    auto md = arena.Create<BlinnPhong>(1 / roughness);
    //    auto ts = arena.Create<BxDF_TorranceSparrowReflection>(rc, md, fresnel_);
    //    bsdf->AddBxDF(ts);
    //}
    //
    //ret.bsdf = bsdf;

    if(!roughness)
        ret.bsdf = arena.Create<BxDF2BSDF<BxDF_SpecularReflection>>(fresnel_, rc);
    else
    {
        auto md = arena.Create<BlinnPhong>(1 / roughness);
        ret.bsdf = arena.Create<BxDF2BSDF<BxDF_TorranceSparrowReflection>>(rc, md, fresnel_);
    }

    return ret;
}

} // namespace Atrc
