#include <Atrc/Math/Geometry.h>

AGZ_NS_BEG(Atrc::Geometry::Sphere)

namespace
{
    // |o + td|^2 = r^2 => t^2 + Bt + C = 0
    // Returns (B, C)
    std::tuple<Real, Real> Coefs(const Ray &r, Real radius)
    {
        AGZ_ASSERT(ValidDir(r));
        return {
            2.0 * Dot(r.direction, r.origin),
            r.origin.LengthSquare() - radius * radius
        };
    }
}

bool HasIntersection(const Ray &r, Real radius)
{
    auto [B, C] = Coefs(r, radius);
    Real delta = B * B - 4.0 * C;
    if(delta < 0.0)
        return false;
    delta = Sqrt(delta);
    
    constexpr Real inv2A = 0.5;
    Real t0 = (-B - delta) * inv2A;
    Real t1 = (-B + delta) * inv2A;
    
    return r.minT <= t0 && t0 <= r.maxT ||
           r.minT <= t1 && t1 <= r.maxT;
}

bool EvalIntersection(const Ray &r, Real radius, Intersection *inct)
{
    AGZ_ASSERT(inct && ValidDir(r));
    
    auto [B, C] = Coefs(r, radius);
    Real delta = B * B - 4.0 * C;
    if(delta < 0.0)
        return false;
    delta = Sqrt(delta);
    
    constexpr auto inv2A = 0.5;
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
    
    Vec3r pos = r.At(t);
    
    Real phi = (!pos.x && !pos.y) ?
                0.0 : Arctan2(pos.y, pos.x);
    if(phi < 0.0)
        phi += 2.0 * PI<Real>;
    
    Real sinTheta = Clamp(pos.z / radius, -1.0, 1.0);
    Real theta = Arcsin(sinTheta);
    
    Real u = 0.5 * InvPI<Real> * phi;
    Real v = InvPI<Real> * theta + 0.5;
    
    inct->wr  = -r.direction;
    inct->pos = pos;
    inct->nor = inct->pos.Normalize();
    inct->t   = t;
    inct->uv  = Vec2r(u, v);
    
    return true;
}

AGZ_NS_END(Atrc::Geometry::Sphere)
