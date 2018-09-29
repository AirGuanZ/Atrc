#pragma once

#include "AGZMath.h"
#include "Ray.h"

AGZ_NS_BEG(Atrc)

// Local geometry information of a parameterized surface point
class SurfaceLocal
{
public:

    Vec3r position;
    Vec2r uv;
    Vec3r normal;
    Vec3r dpdu;
    Vec3r dpdv;

    Vec3r ex, ey, ez;

    SurfaceLocal() = default;

    SurfaceLocal(
        const Vec3r &pos,
        const Vec2r &uv,
        const Vec3r &nor,
        const Vec3r &dpdu, const Vec3r &dpdv)
        : position(pos),
          uv(uv),
          normal(nor),
          dpdu(dpdu),
          dpdv(dpdv),
          ex(AGZ::UNINITIALIZED),
          ey(AGZ::UNINITIALIZED),
          ez(AGZ::UNINITIALIZED)
    {
        ex = dpdu.Normalize();
        ez = normal.Normalize();
        ey = Cross(ez, ex);
    }
};

AGZ_NS_END(Atrc)
