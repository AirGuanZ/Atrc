#include <Atrc/Geometry/Sphere.h>

AGZ_NS_BEG(Atrc)

namespace
{
    // |o + td|^2 = r^2
    //  => At^2 + Bt + C = 0
    std::tuple<Real, Real, Real> ABC(Real radius, const Ray &r)
    {
        Real A = r.dir.LengthSquare();
        Real B = 2.0 * Dot(r.dir, r.ori);
        Real C = r.ori.LengthSquare() - radius * radius;
        return { A, B, C };
    }
}

bool Sphere::HasIntersection(const Ray &_r) const
{
    Ray r = local2World_.ApplyInverseToRay(_r);

    auto [A, B, C] = ABC(radius_, r);
    Real delta = B * B - 4.0 * A * C;
    if(delta < 0.0)
        return false;
    delta = Sqrt(delta);

    Real inv2A = 0.5 / A;
    Real t0 = (-B - delta) * inv2A;
    Real t1 = (-B + delta) * inv2A;

    return r.Between(t0) || r.Between(t1);
}

bool Sphere::FindIntersection(const Ray &_r, SurfacePoint *sp) const
{
    Ray r = local2World_.ApplyInverseToRay(_r);

    // 联立r和sphere方程解出两个t，选更近的那个

    auto [A, B, C] = ABC(radius_, r);
    Real delta = B * B - 4.0 * A * C;
    if(delta < 0.0)
        return false;
    delta = Sqrt(delta);

    Real inv2A = 0.5 / A;
    Real t0 = (-B - delta) * inv2A;
    Real t1 = (-B + delta) * inv2A;

    AGZ_ASSERT(A > 0.0);
    if(r.maxT < t0 || t1 < r.minT)
        return false;
    bool notT0 = t0 < r.minT;
    if(notT0 && t1 > r.maxT)
        return false;
    Real t = notT0 ? t1 : t0;

    // 计算uv坐标

    Vec3 pos = r.At(t);

    Real phi = (!pos.x && !pos.y) ? 0.0 : Arctan2(pos.y, pos.x);
    if(phi < 0.0)
        phi += 2.0 * PI;

    Real sinTheta = Clamp(pos.z / radius_, -1.0, 1.0);
    Real theta = Arcsin(sinTheta);

    Vec2 geoUV(0.5 * InvPI * phi, InvPI * theta + 0.5);

    // 计算local coordinate system

    Vec3 nor = pos;
    Vec3 ex, ey;
    if(ApproxEq(nor.z, 1.0, 1e-5))
    {
        nor = Vec3::UNIT_Z();
        ex = Vec3::UNIT_X();
        ey = Vec3::UNIT_Y();
    }
    else if(ApproxEq(nor.z, -1.0, 1e-5))
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

    sp->t        = t;
    sp->pos      = local2World_.ApplyToPoint(pos) ;
    sp->wo       = -_r.dir.Normalize();
    sp->geoUV    = geoUV;
    sp->usrUV    = geoUV;
    sp->geoLocal = local2World_.ApplyToCoordSystem({ ex, ey, nor });

    return true;
}

Real Sphere::SurfaceArea() const
{
    Real sR = local2World_.ScaleFactor() * radius_;
    return 4.0 * PI * sR * sR;
}

AABB Sphere::LocalBound() const
{
    return { Vec3(-radius_), Vec3(radius_) };
}

GeometrySampleResult Sphere::Sample() const
{
    Real u = Rand(), v = Rand();
    auto [sam, pdf] = AGZ::Math::DistributionTransform
                      ::UniformOnUnitSphere<Real>::Transform({ u, v });

    GeometrySampleResult ret;
    ret.pos = local2World_.ApplyToPoint(radius_ * sam);
    ret.nor = ret.pos.Normalize();

    Real r = radius_ * local2World_.ScaleFactor();
    ret.pdf = pdf / (r * r);

    AGZ_ASSERT(ApproxEq(ret.pdf, SamplePDF(ret.pos), 1e-5));

    return ret;
}

Real Sphere::SamplePDF(const Vec3 &) const
{
    Real r = radius_ * local2World_.ScaleFactor();
    return 1 / (4 * PI * r * r);
}

AGZ_NS_END(Atrc)
