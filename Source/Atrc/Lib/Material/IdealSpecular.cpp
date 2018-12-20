#include <Atrc/Lib/Material/BxDF/BxDF_Specular.h>
#include <Atrc/Lib/Material/IdealSpecular.h>
#include <Atrc/Lib/Core/Texture.h>
#include  <Atrc/Lib/Material/Utility/BxDFAggregate.h>
#include <Atrc/Lib/Material/Utility/Fresnel.h>

namespace Atrc
{

IdealSpecular::IdealSpecular(const Texture *rcMap, const Fresnel *fresnel) noexcept
    : rcMap_(rcMap), fresnel_(dynamic_cast<const Dielectric*>(fresnel))
{
    
}

ShadingPoint IdealSpecular::GetShadingPoint(const Intersection &inct, Arena &arena) const
{
    ShadingPoint ret;
    ret.uv = inct.usr.uv;
    ret.coordSys = inct.usr.coordSys;

    Spectrum rc = rcMap_->Sample(ret.uv);
    auto bsdf   = arena.Create<BxDFAggregate<1>>(ret.coordSys, inct.coordSys);
    BxDF *bxdf  = arena.Create<BxDF_Specular>(rc, fresnel_);
    bsdf->AddBxDF(bxdf);
    ret.bsdf = bsdf;

    return ret;
}

} // namespace Atrc
