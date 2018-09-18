#include "Sphere.h"

AGZ_NS_BEG(Atrc)

namespace
{
    // |o + td|^2 = r^2 => At^2 + Bt + C = 0
    // returns (A, B, C)
    std::tuple<Real, Real, Real> GetRayInctEquCoefs(Real radius, const Ray &ray)
    {
        Real A = LengthSquare(ray.direction);
        Real B = Real(2.0) * Dot(ray.direction, ray.origin);
        Real C = LengthSquare(ray.origin) - radius * radius;
        return { A, B, C };
    }
}

bool Sphere::HasIntersection(const Ray &_ray) const
{
    Ray ray = local2World_->ApplyInverseToRay(_ray);
    auto [A, B, C] = GetRayInctEquCoefs(radius_, ray);

    Real delta = B * B - Real(4.0) * A * C;
    if(delta < Real(0.0))
        return false;
    delta = Sqrt(delta);

    Real inv2A = Real(0.5) / A;
    Real t0 = (-B + delta) * inv2A;
    Real t1 = (-B - delta) * inv2A;

    return ray.minT <= t0 && t0 <= ray.maxT ||
           ray.minT <= t1 && t1 <= ray.maxT;
}

Option<GeometryIntersection> Sphere::EvalIntersection(const Ray &_ray) const
{
    Ray ray = local2World_->ApplyInverseToRay(_ray);
    auto [A, B, C] = GetRayInctEquCoefs(radius_, ray);

    Real delta = B * B - Real(4.0) * A * C;
    if(delta < Real(0.0))
        return None;
    delta = Sqrt(delta);

    Real inv2A = Real(0.5) / A;
    auto [t0, t1] = std::minmax((-B + delta) * inv2A, (-B - delta) * inv2A);

    if(ray.maxT < t0 || t1 < ray.minT)
        return None;
    Real t;
    if(t0 < ray.minT)
    {
        if(t1 <= ray.maxT)
            t = t1;
        else
            return None;
    }
    else
        t = t0;

    Vec3r p = ray.At(t);
    if(p.x == Real(0.0) && p.y == Real(0.0))
        p.x = Real(1e-5) * radius_;

    Real phi = (!p.x && !p.y) ? Real(0.0) : Arctan2(p.y, p.x);
    if(phi < 0.0)
        phi += Real(2.0) * PIr;

    Real sinTheta = Clamp(p.z / radius_, Real(-1.0), Real(1.0));
    Real theta = Arcsin(Clamp(p.z / radius_, Real(-1.0), Real(1.0)));

    Real u = Real(0.5) * InvPIr * phi;
    Real v = InvPIr * theta + Real(0.5);

    Real cosTheta = Cos(theta), cosPhi = Cos(phi), sinPhi = Sin(phi);
    Real radiusPI = radius_ * PIr;

    Vec3r dpdu(-Real(2.0) * PIr * p.y,
               Real(2.0) * PIr * p.x,
               Real(0.0));
    Vec3r dpdv(-radiusPI * sinTheta * cosPhi,
               -radiusPI * sinTheta * sinPhi,
                radiusPI * cosTheta);

    Vec3r normal;
    if(theta > PIr / Real(2.0) - Real(1e-4))
        normal = Vec3r::UNIT_Z();
    else if(theta < -PIr / Real(2.0) + Real(1e-4))
        normal = -Vec3r::UNIT_Z();
    else
        normal = Normalize(Cross(dpdu, dpdv));

    return GeometryIntersection{
        t,
        local2World_->ApplyToSurfaceLocal(SurfaceLocal(
            p,{ u, v }, normal, dpdu, dpdv))
    };
}

Sphere::Sphere(Real radius, const Transform *local2World)
    : GeometryObjectWithTransform(local2World), radius_(radius)
{

}

AGZ_NS_END(Atrc)
