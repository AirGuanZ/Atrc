#pragma once

#include "AGZMath.h"

AGZ_NS_BEG(Atrc)

class Ray
{
public:

    Vec3r origin;
    Vec3r direction;

    Ray() = default;

    Ray(const Vec3d &ori, const Vec3d &dir)
        : origin(ori), direction(dir)
    {

    }

    Vec3d At(Real t) const
    {
        return origin + t * direction;
    }

    void Normalize()
    {
        direction = AGZ::Math::Normalize(direction);
    }
};

AGZ_NS_END(Atrc)
