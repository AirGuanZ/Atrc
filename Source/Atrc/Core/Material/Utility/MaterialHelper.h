#pragma once

#include <Atrc/Core/Core/SurfacePoint.h>
#include "NormalMapper.h"

namespace Atrc::MatHelper
{
    
inline CoordSystem ComputeShadingCoordSystem(const Intersection &inct, const NormalMapper *normalMapper) noexcept
{
    Vec3 localNormal = normalMapper->GetLocalNormal(inct.usr.uv);
    return inct.usr.coordSys.RotateToNewEz(inct.usr.coordSys.Local2World(localNormal));
}

} // namespace Atrc::MatHelper
