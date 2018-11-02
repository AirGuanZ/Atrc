#pragma once

#include <Atrc/Core/Common.h>
#include <Atrc/Core/Ray.h>

AGZ_NS_BEG(Atrc)

class AABB
{
public:

    Vec3 low  = Vec3(RealT::Max());
    Vec3 high = Vec3(RealT::Min());

    // 若(high - low)的任意一个维度<=0，则视为Entity
    bool IsEmpty() const
    {
        return low.x >= high.x || low.y >= high.y || low.z >= high.z;
    }

    Real SurfaceArea() const
    {
        Vec3 delta = high - low;
        if(delta.x <= 0 || delta.y <= 0 || delta.z <= 0)
            return 0;
        return 2 * (delta.x * delta.y + delta.y * delta.z + delta.z * delta.x);
    }

    AABB &Expand(const Vec3 &p)
    {
        for(int i = 0; i < 3; ++i)
        {
            low[i] = Min(low[i], p[i]);
            high[i] = Max(high[i], p[i]);
        }
        return *this;
    }

	/*bool HasIntersection(const Ray &r, const Vec3 &invDir) const
	{
		Real t0 = r.minT, t1 = r.maxT;

		Real n = invDir.x * (low.x - r.ori.x);
		Real f = invDir.x * (high.x - r.ori.x);
		if(n > f)
			std::swap(n, f);
		f *= (1 + 1e-5);
		t0 = Max(n, t0);
		t1 = Min(f, t1);
		if(t0 > t1)
			return false;

		n = invDir.y * (low.y - r.ori.y);
		f = invDir.y * (high.y - r.ori.y);
		if(n > f)
			std::swap(n, f);
		f *= (1 + 1e-5);
		t0 = Max(n, t0);
		t1 = Min(f, t1);
		if(t0 > t1)
			return false;

		n = invDir.z * (low.z - r.ori.z);
		f = invDir.z * (high.z - r.ori.z);
		if(n > f)
			std::swap(n, f);
		f *= (1 + 1e-5);
		t0 = Max(n, t0);
		t1 = Min(f, t1);

		return t0 <= t1;
	}*/

	bool HasIntersection(const Ray &r, const Vec3 &invDir) const
	{
		Real t0 = r.minT, t1 = r.maxT;

		using namespace AGZ::Math;

		Vec3 n = invDir * (low - r.ori);
		Vec3 f = invDir * (high - r.ori);
		t0 = Max(t0, Min(n.x, f.x));
		t0 = Max(t0, Min(n.y, f.y));
		t0 = Max(t0, Min(n.z, f.z));
		t1 = Min(t1, Max(n.x, f.x));
		t1 = Min(t1, Max(n.y, f.y));
		t1 = Min(t1, Max(n.z, f.z));

		return t0 <= t1;
	}

    bool HasIntersection(const Ray &r) const
    {
        Real t0 = r.minT, t1 = r.maxT;

        Real invDir = 1 / r.dir.x;
        Real n = invDir * (low.x - r.ori.x);
        Real f = invDir * (high.x - r.ori.x);
        if(n > f)
            std::swap(n, f);
        f *= Real(1 + 1e-5);
        t0 = Max(n, t0);
        t1 = Min(f, t1);
        if(t0 > t1)
            return false;

        invDir = 1 / r.dir.y;
        n = invDir * (low.y - r.ori.y);
        f = invDir * (high.y - r.ori.y);
        if(n > f)
            std::swap(n, f);
        f *= Real(1 + 1e-5);
        t0 = Max(n, t0);
        t1 = Min(f, t1);
        if(t0 > t1)
            return false;

        invDir = 1 / r.dir.z;
        n = invDir * (low.z - r.ori.z);
        f = invDir * (high.z - r.ori.z);
        if(n > f)
            std::swap(n, f);
        f *= Real(1 + 1e-5);
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
