#include <Atrc/Core/Material/BxDF/BxDF_Void.h>
#include <Atrc/Core/Material/InvisibleSurface.h>
#include <Atrc/Core/Material/Utility/BxDFAggregate.h>

namespace Atrc
{

ShadingPoint InvisibleSurface::GetShadingPoint(const Intersection &inct, Arena &arena) const
{
    static const BxDF_Void BXDF_VOID;

    ShadingPoint ret;
    ret.uv = inct.usr.uv;
    ret.coordSys = inct.coordSys;
    
    auto bsdf = arena.Create<BxDFAggregate<1>>();
    bsdf->AddBxDF(&BXDF_VOID);
    ret.bsdf = bsdf;

    return ret;
}

} // namespace Atrc
