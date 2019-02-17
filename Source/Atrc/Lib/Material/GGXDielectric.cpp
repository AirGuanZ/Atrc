#include <Atrc/Lib/Material/BxDF/BxDF_Specular.h>
#include <Atrc/Lib/Material/BxDF/BxDF_Microfacet.h>
#include <Atrc/Lib/Material/GGXDielectric.h>
#include <Atrc/Lib/Material/Utility/BxDFAggregate.h>
#include <Atrc/Lib/Material/Utility/Fresnel.h>
#include <Atrc/Lib/Material/Utility/MaterialHelper.h>

namespace Atrc
{

GGXDielectric::GGXDielectric(
    const Dielectric *dielectric,
    const Texture *rc, const Texture *roughness, const NormalMapper *normalMapper) noexcept
    : dielectric_(dielectric), rc_(rc), roughness_(roughness), normalMapper_(normalMapper)
{
    AGZ_ASSERT(dielectric && rc && roughness && normalMapper);
}

ShadingPoint GGXDielectric::GetShadingPoint(const Intersection &inct, Arena &arena) const
{
    ShadingPoint ret;
    ret.uv = inct.usr.uv;
    ret.coordSys = MatHelper::ComputeShadingCoordSystem(inct, normalMapper_);

    Spectrum rc = rc_->Sample(ret.uv);
    Real roughness = roughness_->Sample1(ret.uv);

    auto bsdf = arena.Create<BxDFAggregate<1>>(ret.coordSys, inct.coordSys);

    if(roughness < Real(0.04))
    {
        auto ms = arena.Create<BxDF_Specular>(rc, dielectric_);
        bsdf->AddBxDF(ms);
    }
    else
    {
        auto mr = arena.Create<BxDF_Microfacet>(rc, roughness, dielectric_);
        bsdf->AddBxDF(mr);
    }

    ret.bsdf = bsdf;
    return ret;
}

} // namespace Atrc
