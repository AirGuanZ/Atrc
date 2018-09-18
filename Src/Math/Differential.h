#pragma once

#include <type_traits>

#include "AGZMath.h"
#include "Ray.h"

AGZ_NS_BEG(Atrc)

// Local geometry information of a parameterized surface point
class SurfaceLocal
{
public:

    Vec3r position;
    Vec3r normal;
    Vec2r uv;
    Vec3r dpdu;
    Vec3r dpdv;

    SurfaceLocal() = default;

    SurfaceLocal(
        const Vec3r &pos,
        const Vec2r &uv,
        const Vec3r &nor,
        const Vec3r &dpdu, const Vec3r &dpdv)
        : position(pos),
          normal(nor),
          uv(uv),
          dpdu(dpdu),
          dpdv(dpdv)
    {

    }
};

AGZ_NS_END(Atrc)
