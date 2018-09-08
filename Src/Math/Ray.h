#pragma once

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

    Ray() = default;

    Ray(const Vec3r &ori, const Vec3r &dir)
        : origin(ori), direction(dir)
    {

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

class RayT : public Ray
{
public:

    Real t;

    RayT()
        : RayT(0.0)
    {
        
    }

    explicit RayT(Real t)
        : t(t)
    {

    }

    RayT(const Vec3r &ori, const Vec3r &dir, Real t)
        : Ray(ori, dir), t(t)
    {

    }

    Vec3r At(Real t) const
    {
        return Ray::At(t);
    }

    Vec3r At() const
    {
        return At(t);
    }

    bool IsNonNegative() const
    {
        return t >= Real(0.0);
    }
};

class RayR : public RayT
{
public:

    Real min, max;

    RayR()
        : min(0.0), max(0.0)
    {
        
    }

    RayR(Real min, Real max, Real t)
        : RayT(t), min(min), max(max)
    {
        
    }

    RayR(const Vec3r &ori, const Vec3r &dir, Real min, Real max, Real t)
        : RayT(ori, dir, t), min(min), max(max)
    {

    }

    bool IsInRange() const
    {
        return min <= t && t <= max;
    }
};

AGZ_NS_END(Atrc)
