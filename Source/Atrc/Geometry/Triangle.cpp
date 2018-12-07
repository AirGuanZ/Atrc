#include <Atrc/Geometry/Triangle.h>

AGZ_NS_BEG(Atrc)

namespace
{
    Vec3 ComputeNormalizedDpdu(
        const Vec3 &A, const Vec3 &B, const Vec3 &C,
        const Vec2 &a, const Vec2 &b, const Vec2 c,
        const Vec3 &normal)
    {
        Real m00 = b.u - a.u, m01 = b.v - a.v;
        Real m10 = c.u - a.u, m11 = c.v - a.v;
        Real det = m00 * m11 - m01 * m10;

        if(!det)
            return LocalCoordSystem::FromEz(normal).ex;
        
        Real invDet = 1 / det;
        Vec3 dpdu = m11 * invDet * (B - A) - m01 * invDet * (C - A);
        
        return dpdu.Normalize();
    }
}

Triangle::Triangle(
    const Transform &local2World,
    const Vec3 &A, const Vec3 &B, const Vec3 &C,
    const Vec2 &tA, const Vec2 &tB, const Vec2 &tC)
    : Geometry(local2World),
      A(A), B_A(B - A), C_A(C - A),
      tA(tA), tB_tA(tB - tA), tC_tA(tC - tA)
{
    ez = Cross(B_A, C_A).Normalize();
    ex = ComputeNormalizedDpdu(
        A, B, C, tA, tB, tC, ez);
    surfaceArea_ = Cross(B_A, C_A).Length();
}

bool Triangle::HasIntersection(const Ray &_r) const
{
    Ray r = local2World_.ApplyInverseToRay(_r);

    Vec3 s1 = Cross(r.dir, C_A);
    Real div = Dot(s1, B_A);
    if(!div)
        return false;
    Real invDiv = 1 / div;

    Vec3 o_A = r.ori - A;
    Real alpha = Dot(o_A, s1) * invDiv;
    if(alpha < 0)
        return false;

    Vec3 s2 = Cross(o_A, B_A);
    Real beta = Dot(r.dir, s2) * invDiv;
    if(beta < 0 || alpha + beta > 1)
        return false;

    Real t = Dot(C_A, s2) * invDiv;
    return r.Between(t);
}

bool Triangle::FindIntersection(const Ray &_r, SurfacePoint *sp) const
{
    AGZ_ASSERT(sp);
    Ray r = local2World_.ApplyInverseToRay(_r);

    Vec3 s1 = Cross(r.dir, C_A);
    Real div = Dot(s1, B_A);
    if(!div)
        return false;
    Real invDiv = 1 / div;

    Vec3 o_A = r.ori - A;
    Real alpha = Dot(o_A, s1) * invDiv;
    if(alpha < 0)
        return false;

    Vec3 s2 = Cross(o_A, B_A);
    Real beta = Dot(r.dir, s2) * invDiv;
    if(beta < 0 || alpha + beta > 1)
        return false;

    Real t = Dot(C_A, s2) * invDiv;
    if(!r.Between(t))
        return false;

    sp->t     = t;
    sp->geoUV = Vec2(alpha, beta);
    sp->pos   = local2World_.ApplyToPoint(r.At(t));
    sp->wo    = -_r.dir.Normalize();
    sp->usrUV = tA + alpha * tB_tA + beta * tC_tA;
    
    if(Dot(r.dir, ez) < 0)
        sp->geoLocal = local2World_.ApplyToCoordSystem({ ex, Cross(ez, ex), ez });
    else
        sp->geoLocal = local2World_.ApplyToCoordSystem({ ex, -Cross(ez, ex), -ez });

    return true;
}

Real Triangle::SurfaceArea() const
{
    return surfaceArea_;
}

AABB Triangle::LocalBound() const
{
    AABB ret;
    ret.Expand(A).Expand(A + B_A).Expand(A + C_A);
    return ret;
}

GeometrySampleResult Triangle::Sample() const
{
    Real u0 = Rand(), u1 = Rand();
    auto uv = AGZ::Math::DistributionTransform
        ::UniformOnTriangle<Real>::Transform({ u0, u1 });
    
    GeometrySampleResult ret;
    ret.pos = local2World_.ApplyToPoint(A + uv.u * B_A + uv.v * C_A);
    ret.nor = local2World_.ApplyToNormal(Rand() < 0.5 ? ez : -ez);
    ret.pdf = 1 / SurfaceArea();

    return ret;
}

Real Triangle::SamplePDF([[maybe_unused]] const Vec3 &pos) const
{
    return 1 / SurfaceArea();
}

AGZ_NS_END(Atrc)
