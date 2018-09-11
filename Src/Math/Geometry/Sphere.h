#pragma once

#include <tuple>

#include "../../Common.h"
#include "../AGZMath.h"
#include "../Geometry.h"

AGZ_NS_BEG(Atrc)

// x = r * cos(PI * (u - 0.5)) * cos(2PI * v) =   r * sin(PI * u) * cos(2PI * v)
// y = r * cos(PI * (u - 0.5)) * sin(2PI * v) =   r * sin(PI * u) * sin(2PI * v)
// z = r * sin(PI * (u - 0.5))                = - r * cos(PI * u)
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

        Real theta = Arcsin(Clamp(p.z / radius_, Real(-1.0), Real(1.0)));
        Real phi = (!p.x && !p.y) ? Real(0.0) : Arctan2(p.y, p.x);

        Real u = InvPIr * theta + Real(0.5);
        Real v = Real(0.5) * InvPIr * phi;

        Real cosPIu = Cos(PIr * u), sinPIu = Sin(PIr * u);
        Real cosPhi = Cos(phi), sinPhi = Sin(phi);
        Real radiusPI = radius_ * PIr;

        Vec3r dpdu(radiusPI * cosPIu * cosPhi, radiusPI * cosPIu * sinPhi, radiusPI * sinPIu);
        Vec3r dpdv(- Real(2.0) * PIr * p.y, Real(2.0) * PIr * p.x, Real(0.0));

        Vec3r normal;
        if(theta > PIr / Real(2.0) - Real(1e-4))
            normal = Vec3r::UNIT_Z();
        else if(theta < -PIr / Real(2.0) + Real(1e-4))
            normal = -Vec3r::UNIT_Z();
        else
            normal = Normalize(Cross(dpdu, dpdv));

        // Use Weingarten Equations to compute dndu and dndv
        // See https://en.wikipedia.org/wiki/Weingarten_equations
        Real E = Dot(dpdu, dpdu);
        Real F = Dot(dpdu, dpdv);
        Real G = Dot(dpdv, dpdv);
        Real L = Dot(normal, -PIr * PIr * p);
        Real M = Dot(normal, Vec3r(Real(2.0) * PIr * PIr * sinPhi,
                                  -Real(2.0)*  PIr * PIr * cosPhi,
                                  Real(0.0)));
        Real N = Dot(normal, Vec3r(-Real(4.0) * PIr * PIr * p.xy(), Real(0.0)));
        Real invDem = Real(1.0) / (E * G - F * F);

        Vec3r dndu = (F * M - G * L) * invDem * dpdu + (F * L - E * M) * invDem * dpdv;
        Vec3r dndv = (F * N - G * M) * invDem * dpdu + (F * M - E * N) * invDem * dpdv;

        return Intersection {
            t,
            local2World_->ApplyToSurfaceLocal(SurfaceLocal(
                p, { u, v }, normal, dpdu, dpdv, dndu, dndv))
        };
    }
};

AGZ_NS_END(Atrc)
