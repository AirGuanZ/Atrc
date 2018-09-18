#pragma once

#include <tuple>

#include "../../Common.h"
#include "../AGZMath.h"
#include "GeometryObjectWithTransform.h"

AGZ_NS_BEG(Atrc)

// x = r * cos(PI * (u - 0.5)) * cos(2PI * v)
// y = r * cos(PI * (u - 0.5)) * sin(2PI * v)
// z = r * sin(PI * (u - 0.5))
class Sphere : public GeometryObjectWithTransform
{
    Real radius_;

public:

    explicit Sphere(
        Real radius,
        const Transform *local2World = &Transform::StaticIdentity());

    bool HasIntersection(const Ray &_ray) const override;

    Option<GeometryIntersection> EvalIntersection(const Ray &_ray) const override;

    Real GetRadius() const { return radius_; }
};

AGZ_NS_END(Atrc)
