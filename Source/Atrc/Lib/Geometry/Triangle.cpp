#include <Atrc/Lib/Geometry/Triangle.h>
#include <Atrc/Lib/Utility/TriangleAux.h>

namespace Atrc
{
    
Triangle::Triangle(
    const Transform &local2World, const Vec3 &A, const Vec3 &B, const Vec3 &C,
    const Vec2 &tA, const Vec2 &tB, const Vec2 &tC) noexcept
    : Geometry(local2World), A(A), B_A(B - A), C_A(C - A),
      tA(tA), tB_tA(tB - tA), tC_tA(tC - tA)
{
    nor = Cross(B_A, C_A).Normalize();
    ex = ComputeDpduAsEx(B_A, C_A, tB_tA, tC_tA, nor);
}

bool Triangle::HasIntersection(const Ray &r) const noexcept
{
    return HasIntersectionWithTriangle(local2World_.ApplyInverseToRay(r), A, B_A, C_A);
}

bool Triangle::FindIntersection(const Ray &r, GeometryIntersection *inct) const noexcept
{
    AGZ_ASSERT(inct);

    Ray lRay = local2World_.ApplyInverseToRay(r);
    TriangleIntersectionRecoed rc;
    if(!FindIntersectionWithTriangle(lRay, A, B_A, C_A, &rc))
        return false;

    Vec3 ez = Dot(-lRay.d, nor) > 0 ? nor : -nor;

    inct->t            = rc.t;
    inct->pos          = r.At(rc.t);
    inct->wr           = -r.d;
    inct->uv           = rc.uv;
    inct->coordSys     = local2World_.ApplyToCoordSystem(CoordSystem(ex, Cross(ez, ex), ez));
    inct->usr.uv       = tA + rc.uv.u * tB_tA + rc.uv.v * tC_tA;
    inct->usr.coordSys = inct->coordSys;

    return true;
}

Real Triangle::GetSurfaceArea() const noexcept
{
    return local2World_.ScaleFactor() * local2World_.ScaleFactor() * 2 * GetTriangleArea(B_A, C_A);
}

AABB Triangle::GetLocalBound() const noexcept
{
    AABB ret;
    ret.Expand(A).Expand(B_A).Expand(C_A);
    for(int i = 0; i < 3; ++i)
    {
        if(ret.high[i] <= ret.low[i])
        {
            Real s = Real(0.01) * Max(Abs(ret.high[i]), Abs(ret.low[i]));
            ret.high[i] += s;
            ret.low[i]  -= s;
        }
    }
    return ret;
}

AABB Triangle::GetWorldBound() const noexcept
{
    AABB ret;
    ret.Expand(local2World_.ApplyToPoint(A))
       .Expand(local2World_.ApplyToPoint(B_A))
       .Expand(local2World_.ApplyToPoint(C_A));
    for(int i = 0; i < 3; ++i)
    {
        if(ret.high[i] <= ret.low[i])
        {
            Real s = Real(0.01) * Max(Abs(ret.high[i]), Abs(ret.low[i]));
            ret.high[i] += s;
            ret.low[i]  -= s;
        }
    }
    return ret;
}

Geometry::SampleResult Triangle::Sample(const Vec3 &sample) const noexcept
{
    auto uv = AGZ::Math::DistributionTransform
                ::UniformOnTriangle<Real>::Transform(sample.xy());

    SampleResult ret;
    ret.pos = local2World_.ApplyToPoint(A + uv.u * B_A + uv.v * C_A);
    ret.nor = local2World_.ApplyToVector(sample.z < Real(0.5) ? nor : -nor).Normalize();
    ret.pdf = 1 / GetSurfaceArea();

    return ret;
}

Real Triangle::SamplePDF([[maybe_unused]] const Vec3 &pos) const noexcept
{
    return 1 / GetSurfaceArea();
}

} // namespace Atrc
