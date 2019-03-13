#include <Atrc/Lib/Material/BxDF/BxDF_Void.h>
#include <Atrc/Lib/Material/InvisibleSurface.h>
#include <Atrc/Lib/Material/Utility/BxDFAggregate.h>

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
