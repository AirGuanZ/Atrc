#pragma once

#include <Atrc/Common.h>

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

    Ray(const Vec3r &ori, const Vec3r &dir, Real minT = 0.0, Real maxT = RealT::Infinity())
        : origin(ori), direction(dir), minT(minT), maxT(maxT)
    {
        AGZ_ASSERT(ApproxEq(dir.Length(), 1.0, 1e-5));
    }

    Vec3r At(Real t) const
    {
        return origin + t * direction;
    }

    Ray Normalize() const
    {
        return Ray(origin, direction.Normalize(), minT, maxT);
    }
};

inline bool ValidDir(const Ray &r)
{
    return ApproxEq(r.direction.LengthSquare(), 1.0, 1e-5);
}

AGZ_NS_END(Atrc)
