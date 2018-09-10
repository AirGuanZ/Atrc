#pragma once

#include <tuple>

#include "../../Common.h"
#include "../AGZMath.h"
#include "../Geometry.h"

AGZ_NS_BEG(Atrc)

// u: theta [0, 2 * PI], v: phi [0, PI]
// x = r * sin(u) * cos(v)
// y = r * sin(u) * sin(phi)
// z = r * cos(theta)
class Sphere : public GeometryObjectWithTransform
{
    Real radius_;

    std::tuple<Real, Real, Real> GetRayInctEquCoefs(const Ray &ray) const
    {
        return std::make_tuple<Real, Real, Real>(
            LengthSquare(ray.direction),
            Real(2.0) * Dot(ray.direction, ray.origin),
            radius_ * radius_ - LengthSquare(ray.origin)
        );
    }

public:

    Sphere(const Transform *local2World, Real radius)
        : GeometryObjectWithTransform(local2World),
          radius_(radius)
    {

    }

    Real GetRadius() const
    {
        return radius_;
    }

    bool HasIntersection(
        const Ray &_ray,
        Real minT, Real maxT
    ) const override
    {
        Ray ray = local2World_->ApplyInverseToRay(_ray);
        auto [A, B, C] = GetRayInctEquCoefs(ray);

        Real delta = B * B - Real(4.0) * A * C;
        if(delta < Real(0.0))
            return false;
        delta = Sqrt(delta);

        Real inv2A = Real(0.5) / A;
        Real t0 = (-B + delta) * inv2A;
        Real t1 = (-B - delta) * inv2A;

        return (minT <= t0 && t0 <= maxT) ||
               (minT <= t1 && t1 <= maxT);
    }

    Option<Intersection> EvalIntersection(
        const Ray &_ray,
        Real minT, Real maxT)
    const override
    {
        Ray ray = local2World_->ApplyInverseToRay(_ray);
        auto [A, B, C] = GetRayInctEquCoefs(ray);

        Real delta = B * B - Real(4.0) * A * C;
        if(delta < Real(0.0))
            return None;
        delta = Sqrt(delta);

        Real inv2A = Real(0.5) / A;
        Real t0 = (-B + delta) * inv2A;
        Real t1 = (-B - delta) * inv2A;
        if(t0 > t1)
            std::swap(t0, t1);

        if(maxT < t0 || t1 < minT)
            return None;
        Real t;
        if(t0 < minT)
        {
            if(t1 <= maxT)
                t = t1;
            else
                return None;
        }
        else
            t = t0;

        Vec3r p = ray.At(t);
        if(p.x == Real(0.0) && p.y == Real(0.0))
           p.x = Real(1e-5) * radius_;

        Real phi = Arctan2(p.y, p.x);
        if(phi < Real(0.0))
            phi += Real(2.0) * PI<Real>();
        Real theta = Arccos(Clamp(p.z / radius_,
                            Real(-1.0), Real(1.0)));

        Real u = phi / (Real(2.0) * PI<Real>());
        Real v = theta / PI<Real>();

        // TODO

        return None;
    }
};

AGZ_NS_END(Atrc)
