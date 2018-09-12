#pragma once

#include <tuple>

#include "../../Common.h"
#include "../AGZMath.h"
#include "GeometryObjectWithTransform.h"

AGZ_NS_BEG(Atrc)

// x = r * cos(PI * (u - 0.5)) * cos(2PI * v) = + r * sin(PI * v) * cos(2PI * u)
// y = r * cos(PI * (u - 0.5)) * sin(2PI * v) = + r * sin(PI * v) * sin(2PI * u)
// z = r * sin(PI * (u - 0.5))                = - r * cos(PI * v)
class Sphere : public GeometryObjectWithTransform
{
    Real radius_;

protected:

    bool HasIntersectionImpl(
        const Ray &_ray, Real minT, Real maxT
    ) const override;

    Option<Intersection> EvalIntersectionImpl(
        const Ray &_ray, Real minT, Real maxT
    ) const override;

public:

    explicit Sphere(
        Real radius,
        const Transform *local2World = &Transform::StaticIdentity());

    Real GetRadius() const { return radius_; }
};

AGZ_NS_END(Atrc)
