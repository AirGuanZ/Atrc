#pragma once

#include <initializer_list>

#include <Atrc/Common.h>

AGZ_NS_BEG(Atrc)

class AABB
{
public:

    Vec3r low, high;
    
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
