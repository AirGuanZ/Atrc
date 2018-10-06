#pragma once

#include <Atrc/Common.h>
#include <Atrc/Math/Ray.h>

AGZ_NS_BEG(Atrc)

class AABB
{
public:

    Vec3r low = Vec3r(RealT::Max()), high = Vec3r(RealT::Min());

    static AABB EMPTY_AABB()
    {
        return {
            Vec3r(RealT::Max()),
            Vec3r(RealT::Min())
        };
    }

    bool Contains(const AABB &contained) const
    {
        return low.x <= contained.low.x   &&
               low.y <= contained.low.y   &&
               low.z <= contained.low.z   &&
               contained.high.x <= high.x &&
               contained.high.y <= high.y &&
               contained.high.z <= high.z;
    }
    
    void Expand(const Vec3r &p)
    {
        low.x  = Min(p.x, low.x);
        low.y  = Min(p.y, low.y);
        low.z  = Min(p.z, low.z);
        high.x = Max(p.x, high.x);
        high.y = Max(p.y, high.y);
        high.z = Max(p.z, high.z);
    }

    Real SurfaceArea() const
    {
        Vec3r delta = high - low;
        if(delta.x <= 0.0 || delta.y <= 0.0 || delta.z <= 0.0)
            return 0.0;
        return 2 * (delta.x * delta.y + delta.y * delta.z + delta.z * delta.x);
    }

    bool HasIntersection(const Ray &r) const
    {
        Real t0 = r.minT, t1 = r.maxT;
        for(int i = 0; i < 3; ++i)
        {
            if(RealT(r.direction[i]).ApproxEq(0))
            {
                if(r.origin[i] < low[i] || r.origin[i] > high[i])
                    return false;
                continue;
            }

            Real invDir = 1 / r.direction[i];
            Real n = invDir * (low[i] - r.origin[i]);
            Real f = invDir * (high[i] - r.origin[i]);
            if(n > f)
                std::swap(n, f);
            f *= (1 + 1e-5);
            t0 = Max(n, t0);
            t1 = Min(f, t1);
            if(t0 > t1)
                return false;
        }
        return true;
    }
};

inline AABB operator|(const AABB &lhs, const AABB &rhs)
{
    return {
        {
            Min(lhs.low.x, rhs.low.x),
            Min(lhs.low.y, rhs.low.y),
            Min(lhs.low.z, rhs.low.z)
        },
        {
            Max(lhs.high.x, rhs.high.x),
            Max(lhs.high.y, rhs.high.y),
            Max(lhs.high.z, rhs.high.z)
        }
    };
}

inline AABB operator&(const AABB &lhs, const AABB &rhs)
{
    Real xLow  = Max(lhs.low.x, rhs.low.x);
    Real yLow  = Max(lhs.low.y, rhs.low.y);
    Real zLow  = Max(lhs.low.x, rhs.low.x);
    Real xHigh = Max(xLow, Min(lhs.high.x, rhs.high.x));
    Real yHigh = Max(yLow, Min(lhs.high.y, rhs.high.y));
    Real zHigh = Max(zLow, Min(lhs.high.z, rhs.high.z));
    return { { xLow,  yLow,   zLow }, { xHigh, yHigh, zHigh } };
}

AGZ_NS_END(Atrc)
