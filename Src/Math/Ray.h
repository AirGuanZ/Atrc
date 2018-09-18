#pragma once

#include <Utils.h>

#include "../Common.h"
#include "AGZMath.h"

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

AGZ_NS_BEG(Atrc)

class Ray
{
public:

    Vec3r origin;
    Vec3r direction;

    Real minT, maxT;
    uint32_t depth;

    Ray(const Vec3r &ori, const Vec3r &dir, Real minT, Real maxT, uint32_t depth)
        : origin(ori), direction(dir), minT(minT), maxT(maxT), depth(depth)
    {

    }

    static Ray New(const Vec3r &ori, const Vec3r &dir)
    {
        return Ray(ori, dir, Real(0), RealT::Infinity(), 0);
    }

    static Ray NewSegment(const Vec3r &ori, const Vec3r &dir, Real minT, Real maxT = RealT::Infinity())
    {
        return Ray(ori, dir, minT, maxT, 0);
    }

    Ray Child(Real minT = Real(0), Real maxT = RealT::Infinity()) const
    {
        return Ray(origin, direction, minT, maxT, depth + 1);
    }

    Vec3r At(Real t) const
    {
        return origin + t * direction;
    }

    void Normalize()
    {
        direction = AGZ::Math::Normalize(direction);
    }
};

AGZ_NS_END(Atrc)
