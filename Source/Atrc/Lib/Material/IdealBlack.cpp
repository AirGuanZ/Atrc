#include <Atrc/Lib/Core/Entity.h>
#include <Atrc/Lib/Material/BxDF/BxDF_Black.h>
#include <Atrc/Lib/Material/IdealBlack.h>
#include <Atrc/Lib/Material/Utility/BxDFAggregate.h>

namespace Atrc
{

ShadingPoint IdealBlack::GetShadingPoint(const Intersection &inct, Arena &arena) const noexcept
{
    static const BxDF_Black bxdf;

    ShadingPoint ret;
    ret.uv       = inct.entity->GetShadingUV(inct);
    ret.coordSys = inct.entity->GetShadingCoordSys(inct);

    auto bsdf = arena.Create<BxDFAggregate<1>>(ret.coordSys, inct.coordSys);
    bsdf->AddBxDF(&bxdf);
    ret.bsdf     = bsdf;
    return ret;
}

} // namespace Atrc
