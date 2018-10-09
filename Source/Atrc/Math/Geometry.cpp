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
    
    return (r.minT <= t0 && t0 <= r.maxT) ||
           (r.minT <= t1 && t1 <= r.maxT);
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

AGZ_NS_BEG(Atrc::Geometry::Triangle)

bool HasIntersection(const Vec3r &A, const Vec3r &B_A, const Vec3r &C_A, const Ray &r)
{
    Vec3r wr = -r.direction;
    Real detD = Mat3r::FromCols(B_A, C_A, wr).Determinant();
    if(ApproxEq(RealT(detD), RealT(0.0)))
        return false;
    Real invDetD = 1.0 / detD;

    Vec3r o_A = r.origin - A;
    Real t = invDetD * Mat3r::FromCols(B_A, C_A, o_A).Determinant();
    if(t < r.minT || t > r.maxT)
        return false;

    Real alpha = invDetD * Mat3r::FromCols(o_A, C_A, wr).Determinant();
    Real beta  = invDetD * Mat3r::FromCols(B_A, o_A, wr).Determinant();
    return 0.0 <= alpha && 0.0 <= beta && alpha + beta <= 1.0;
}

bool EvalIntersection(
    const Vec3r &A, const Vec3r &B_A, const Vec3r &C_A,
    const Ray &r, Intersection *inct)
{
    AGZ_ASSERT(inct);

    Vec3r s1 = Cross(r.direction, C_A);
    Real div = Dot(s1, B_A);
    if(!div)
        return false;
    Real invDiv = 1 / div;

    Vec3r o_A = r.origin - A;
    Real alpha = Dot(o_A, s1) * invDiv;
    if(alpha < 0 || alpha > 1)
        return false;

    Vec3r s2 = Cross(o_A, B_A);
    Real beta = Dot(r.direction, s2) * invDiv;
    if(beta < 0 || alpha + beta > 1)
        return false;

    Real t = Dot(C_A, s2) * invDiv;
    if(t < r.minT || t > r.maxT)
        return false;

    inct->wr  = -r.direction;
    inct->pos = r.At(t);
    inct->t   = t;
    inct->uv  = Vec2r(alpha, beta);

    return true;
}

AABB ToBoundingBox(const Vec3r &A, const Vec3r &B, const Vec3r &C)
{
    return {
        {
            Min(A.x, Min(B.x, C.x)),
            Min(A.y, Min(B.y, C.y)),
            Min(A.z, Min(B.z, C.z))
        },
        {
            Max(A.x, Max(B.x, C.x)),
            Max(A.y, Max(B.y, C.y)),
            Max(A.z, Max(B.z, C.z))
        }
    };
}

Real SurfaceArea(const Vec3r &A, const Vec3r &B, const Vec3r &C)
{
    return 0.5 * Cross(B - A, C - A).Length();
}

Real SurfaceArea(const Vec3r &B_A, const Vec3r &C_A)
{
    return 0.5 * Cross(B_A, C_A).Length();
}

AGZ_NS_END(Atrc::Geometry::Triangle)
