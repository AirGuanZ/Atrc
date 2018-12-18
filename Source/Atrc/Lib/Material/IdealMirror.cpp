#include <Atrc/Lib/Material/BxDF/BxDF_SpecularReflection.h>
#include <Atrc/Lib/Material/IdealMirror.h>
#include <Atrc/Lib/Core/Texture.h>
#include  <Atrc/Lib/Material/Utility/BxDFAggregate.h>
#include <Atrc/Lib/Material/Utility/Fresnel.h>

namespace Atrc
{

IdealMirror::IdealMirror(const Texture *rcMap, const Fresnel *fresnel) noexcept
    : rcMap_(rcMap), fresnel_(fresnel)
{
    
}

ShadingPoint IdealMirror::GetShadingPoint(const Intersection &inct, Arena &arena) const
{
    ShadingPoint ret;
    ret.uv = inct.usr.uv;
    ret.coordSys = inct.usr.coordSys;

    Spectrum rc = rcMap_->Sample(ret.uv);
    auto bsdf   = arena.Create<BxDFAggregate<1>>(ret.coordSys, inct.coordSys);
    BxDF *bxdf  = arena.Create<BxDF_SpecularReflection>(fresnel_, rc);
    bsdf->AddBxDF(bxdf);
    ret.bsdf = bsdf;

    return ret;
}

} // namespace Atrc
