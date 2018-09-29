#include <Atrc/Math/Sphere.h>

AGZ_NS_BEG(Atrc::Geometry::Sphere)

namespace
{
    // |o + td|^2 = r^2 => t^2 + Bt + C = 0
    // Returns (B, C)
    inline std::tuple<Real, Real> Coefs(const Ray &r, Real radius)
    {
        AGZ_ASSERT(ValidDir(r));
        return {
            Real(2) * Dot(r.direction, r.origin),
            r.origin.LengthSquare() - radius * radius
        };
    }
}

bool HasIntersection(const Ray &r, Real radius)
{
    auto [B, C] = Coefs(r, radius);
    Real delta = B * B - Real(4) * C;
    if(delta < Real(0))
        return false;
    delta = Sqrt(delta);
    
    constexpr Real inv2A = Real(0.5);
    Real t0 = (-B - delta) * inv2A;
    Real t1 = (-B + delta) * inv2A;
    
    return r.minT <= t0 && t0 <= r.maxT ||
           r.minT <= t1 && t1 <= r.maxT;
}

bool EvalIntersection(const Ray &r, Real radius, Intersection *inct)
{
    AGZ_ASSERT(inct && ValidDir(r));
    
    auto [B, C] = Coefs(r, radius);
    Real delta = B * B - Real(4) * C;
    if(delta < Real(0))
        return false;
    delta = Sqrt(delta);
    
    constexpr Real inv2A = Real(0.5);
    Real t0 = (-B - delta) * inv2A;
    Real t1 = (-B + delta) * inv2A;
    
    if(r.maxT < t0 || t1 < r.minT)
        return false;
    Real t;
    if(t0 < r.minT)
    {
        if(t1 > r.maxT)
            return false;
        t = t1;
    }
    else
        t = t0;
    
    inct->wr  = -r.direction;
    inct->pos = r.At(t);
    inct->nor = inct->pos.Normalize();
    inct->t   = t;
    
    return true;
}

AGZ_NS_END(Atrc::Geometry::Sphere)
