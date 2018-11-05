#pragma once

#include <Atrc/Core/Common.h>

AGZ_NS_BEG(Atrc)

class Ray
{
public:

    Vec3 ori, dir;
    Real minT, maxT;

	const Medium *medium;

    Ray(const Vec3 &ori, const Vec3 &dir, Real minT = 0.0, Real maxT = RealT::Max(), const Medium *medium = nullptr)
        : ori(ori), dir(dir), minT(minT), maxT(maxT), medium(medium)
    {
        
    }

    Vec3 At(Real t) const
    {
        return ori + t * dir;
    }

    bool Between(Real t) const
    {
        return minT <= t && t <= maxT;
    }

    bool IsNormalized() const
    {
        return ApproxEq(dir.Length(), Real(1), EPS);
    }
};

AGZ_NS_END(Atrc)
