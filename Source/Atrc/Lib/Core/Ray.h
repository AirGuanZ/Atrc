#pragma once

#include <Atrc/Lib/Core/Common.h>

namespace Atrc
{

class Ray
{
public:

    Vec3 o, d;
    Real t0, t1;

    Ray(
        const Vec3 &o, const Vec3 &d,
        Real minT = EPS, Real maxT = RealT::Infinity()) noexcept;
    
    Vec3 At(Real t) const noexcept;

    bool Between(Real t) const noexcept;
};

// ================================= Implementation

inline Ray::Ray(
    const Vec3 &o, const Vec3 &d, Real minT, Real maxT) noexcept
    : o(o), d(d), t0(minT), t1(maxT)
{

}

inline Vec3 Ray::At(Real t) const noexcept
{
    return o + t * d;
}

inline bool Ray::Between(Real t) const noexcept
{
    return t0 <= t && t <= t1;
}

} // namespace Atrc
