#pragma once

#include <Atrc/Core/Common.h>
#include <Atrc/Core/Ray.h>

AGZ_NS_BEG(Atrc)

class AABB
{
public:

    Vec3 low  = Vec3(RealT::Max());
    Vec3 high = Vec3(RealT::Min());

    bool IsEmpty() const
    {
        return low.x >= high.x || low.y >= high.y || low.z >= high.z;
    }

    Real SurfaceArea() const
    {
        Vec3 delta = high - low;
        if(delta.x <= 0.0 || delta.y <= 0.0 || delta.z <= 0.0)
            return 0.0;
        return 2.0 * (delta.x * delta.y + delta.y * delta.z + delta.z * delta.x);
    }

    AABB &Expand(const Vec3 &p)
    {
        for(int i = 0; i < 3; ++i)
        {
            low[i] = Min(low[i], p[i]);
            high[i] = Max(low[i], p[i]);
        }
        return *this;
    }

    bool HasIntersection(const Ray &r) const
    {
        Real t0 = r.minT, t1 = r.maxT;

        Real invDir = 1 / r.dir.x;
        Real n = invDir * (low.x - r.ori.x);
        Real f = invDir * (high.x - r.ori.x);
        if(n > f)
            std::swap(n, f);
        f *= (1 + 1e-5);
        t0 = Max(n, t0);
        t1 = Min(f, t1);
        if(t0 > t1)
            return false;

        invDir = 1 / r.dir.y;
        n = invDir * (low.y - r.ori.y);
        f = invDir * (high.y - r.ori.y);
        if(n > f)
            std::swap(n, f);
        f *= (1 + 1e-5);
        t0 = Max(n, t0);
        t1 = Min(f, t1);
        if(t0 > t1)
            return false;

        invDir = 1 / r.dir.z;
        n = invDir * (low.z - r.ori.z);
        f = invDir * (high.z - r.ori.z);
        if(n > f)
            std::swap(n, f);
        f *= (1 + 1e-5);
        t0 = Max(n, t0);
        t1 = Min(f, t1);

        return t0 <= t1;
    }

    AABB operator|(const AABB &rhs) const
    {
        AABB ret = { Vec3(AGZ::UNINITIALIZED), Vec3(AGZ::UNINITIALIZED) };
        for(int i = 0; i < 3; ++i)
        {
            ret.low[i] = Min(low[i], rhs.low[i]);
            ret.high[i] = Max(high[i], rhs.high[i]);
        }
        return ret;
    }
};

AGZ_NS_END(Atrc)
