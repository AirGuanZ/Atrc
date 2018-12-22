#include <Atrc/Lib/Geometry/Sphere.h>

namespace Atrc
{

namespace
{
    // |o + td|^2 = r^2
    //  => At^2 + Bt + C = 0
    std::tuple<Real, Real, Real> ABC(Real radius, const Ray &r)
    {
        Real A = r.d.LengthSquare();
        Real B = 2 * Dot(r.d, r.o);
        Real C = r.o.LengthSquare() - radius * radius;
        return { A, B, C };
    }
}

bool Sphere::HasIntersection(const Ray &_r) const noexcept
{
    Ray r = local2World_.ApplyInverseToRay(_r);

    auto [A, B, C] = ABC(radius_, r);
    Real delta = B * B - 4 * A * C;
    if(delta < 0)
        return false;
    delta = Sqrt(delta);

    Real inv2A = Real(0.5) / A;
    Real t0 = (-B - delta) * inv2A;
    Real t1 = (-B + delta) * inv2A;

    return r.Between(t0) || r.Between(t1);
}

bool Sphere::FindIntersection(const Ray &_r, GeometryIntersection *inct) const noexcept
{
    Ray r = local2World_.ApplyInverseToRay(_r);

    // 联立r和sphere方程解出两个t，选更近的那个

    auto [A, B, C] = ABC(radius_, r);
    Real delta = B * B - 4 * A * C;
    if(delta < 0)
        return false;
    delta = Sqrt(delta);

    Real inv2A = Real(0.5) / A;
    Real t0 = (-B - delta) * inv2A;
    Real t1 = (-B + delta) * inv2A;

    AGZ_ASSERT(A > 0);
    if(r.t1 < t0 || t1 < r.t0)
        return false;
    bool notT0 = t0 < r.t0;
    if(notT0 && t1 > r.t1)
        return false;
    Real t = notT0 ? t1 : t0;

    // 计算uv坐标

    Vec3 pos = r.At(t);

    Real phi = (!pos.x && !pos.y) ? Real(0) : Arctan2(pos.y, pos.x);
    if(phi < 0)
        phi += 2 * PI;

    Real sinTheta = Clamp(pos.z / radius_, -Real(1), Real(1));
    Real theta = Arcsin(sinTheta);

    Vec2 uv(Real(0.5) * InvPI * phi, InvPI * theta + Real(0.5));

    // 计算local coordinate system

    Vec3 nor = pos;
    Vec3 ex, ey;
    if(ApproxEq(nor.z, Real(1), EPS))
    {
        nor = Vec3::UNIT_Z();
        ex = Vec3::UNIT_X();
        ey = Vec3::UNIT_Y();
    }
    else if(ApproxEq(nor.z, Real(-1), EPS))
    {
        nor = -Vec3::UNIT_Z();
        ex = Vec3::UNIT_Y();
        ey = Vec3::UNIT_X();
    }
    else
    {
        ex = Cross(Vec3::UNIT_Z(), nor);
        ey = Cross(nor, ex);
    }

    inct->t            = t;
    inct->pos          = local2World_.ApplyToPoint(pos) ;
    inct->wr           = -_r.d;
    inct->uv           = uv;
    inct->coordSys     = local2World_.ApplyToCoordSystem({ ex, ey, nor });
    inct->usr.uv       = uv;
    inct->usr.coordSys = inct->coordSys;

    return true;
}

Real Sphere::GetSurfaceArea() const noexcept
{
    Real sR = local2World_.ScaleFactor() * radius_;
    return 4 * PI * sR * sR;
}

AABB Sphere::GetLocalBound() const noexcept
{
    return { Vec3(-radius_), Vec3(radius_) };
}

Sphere::SampleResult Sphere::Sample(const Vec3 &sample) const noexcept
{
    auto [sam, pdf] = AGZ::Math::DistributionTransform
                      ::UniformOnUnitSphere<Real>::Transform(sample.xy());

    SampleResult ret;
    ret.pos = local2World_.ApplyToPoint(radius_ * sam);
    ret.nor = local2World_.ApplyInverseToVector(sam).Normalize();

    Real r = radius_ * local2World_.ScaleFactor();
    ret.pdf = pdf / (r * r);

    AGZ_ASSERT(ApproxEq(ret.pdf, SamplePDF(ret.pos), EPS));

    return ret;
}

Real Sphere::SamplePDF(const Vec3 &) const noexcept
{
    Real r = radius_ * local2World_.ScaleFactor();
    return 1 / (4 * PI * r * r);
}

Geometry::SampleResult Sphere::Sample(const Vec3 &ref, const Vec3 &sample) const noexcept
{
    Vec3 lref = local2World_.ApplyInverseToPoint(ref);
    Real d = lref.Length();
    if(d <= radius_)
        return Sample(sample);

    Real cosTheta = Min(radius_ / d, Real(1));
    auto [sam, pdf] = AGZ::Math::DistributionTransform::UniformOnCone<Real>
                        ::Transform(cosTheta, sample.xy());

    sam = CoordSystem::FromEz(lref).Local2World(sam).Normalize();
    Real r = radius_ * local2World_.ScaleFactor();

    SampleResult ret;
    ret.nor = local2World_.ApplyToVector(sam).Normalize();
    ret.pos = local2World_.ApplyToPoint(radius_ * sam);
    ret.pdf = pdf / (r * r);

    return ret;
}

Real Sphere::SamplePDF(const Vec3 &pos, const Vec3 &ref) const noexcept
{
    Vec3 lref = local2World_.ApplyInverseToPoint(ref);
    Real d = lref.Length();
    if(d <= radius_)
        return SamplePDF(pos);
    Real cosTheta = Min(radius_ / d, Real(1));
    Real r = radius_ * local2World_.ScaleFactor();
    return AGZ::Math::DistributionTransform::UniformOnCone<Real>::PDF(cosTheta) / (r * r);
}

} // namespace Atrc
