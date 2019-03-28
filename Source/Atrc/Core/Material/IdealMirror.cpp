#include <Atrc/Core/Material/BxDF/BxDF_SpecularReflection.h>
#include <Atrc/Core/Material/IdealMirror.h>
#include <Atrc/Core/Core/Texture.h>
#include <Atrc/Core/Material/Utility/BxDF2BSDF.h>
#include <Atrc/Core/Material/Utility/Fresnel.h>

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
    ret.bsdf = arena.Create<BxDF2BSDF<BxDF_SpecularReflection>>(fresnel_, rc);

    return ret;
}

} // namespace Atrc
