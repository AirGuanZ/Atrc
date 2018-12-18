#pragma once

#include <Atrc/Lib/Core/Ray.h>

namespace Atrc
{

class AABB
{
public:

    Vec3 low  = Vec3(RealT::Max());
    Vec3 high = Vec3(RealT::Min());

    Real SurfaceArea() const noexcept;

    bool HasIntersection(const Ray &r) const noexcept;

    bool HasIntersection(const Ray &r, const Vec3 &invDir) const noexcept;

    AABB &Expand(const Vec3 &p) noexcept;

    AABB &operator|=(const AABB &other) noexcept;
};

// ================================= Implementation

inline Real AABB::SurfaceArea() const noexcept
{
    Vec3 delta = (high - low).Map(AGZ::Math::ClampToPositive<Real>);
    return 2 * (delta.x * delta.y + delta.y * delta.z + delta.z * delta.x);
}

inline bool AABB::HasIntersection(const Ray &r) const noexcept
{
#ifdef AGZ_CC_GCC
    return HasIntersection(r, r.d.Map<decltype(&AGZ::Math::Reciprocate<Real>)>(
                                               &AGZ::Math::Reciprocate<Real>));
#else
    return HasIntersection(r, r.d.Map(AGZ::Math::Reciprocate<Real>));
#endif
}

inline bool AABB::HasIntersection(const Ray &r, const Vec3 &invDir) const noexcept
{
    Real t0 = r.t0, t1 = r.t1;
    Vec3 n = invDir * (low - r.o), f = invDir * (high - r.o);
    t0 = Max(t0, Min(n.x, f.x));
    t0 = Max(t0, Min(n.y, f.y));
    t0 = Max(t0, Min(n.z, f.z));
    t1 = Min(t1, Max(n.x, f.x));
    t1 = Min(t1, Max(n.y, f.y));
    t1 = Min(t1, Max(n.z, f.z));
    return t0 <= t1;
}

inline AABB &AABB::Expand(const Vec3 &p) noexcept
{
    for(int i = 0; i < 3; ++i)
    {
        low[i]  = Min(low[i], p[i]);
        high[i] = Max(high[i], p[i]);
    }
    return *this;
}

inline AABB &AABB::operator|=(const AABB &other) noexcept
{
    for(int i = 0; i < 3; ++i)
    {
        low[i]  = Min(low[i], other.low[i]);
        high[i] = Max(high[i], other.high[i]);
    }
    return *this;
}

}
