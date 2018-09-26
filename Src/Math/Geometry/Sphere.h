#pragma once

#include <Utils/Misc.h>

#include "../../Common.h"
#include "../AGZMath.h"
#include "../Geometry.h"

AGZ_NS_BEG(Atrc)

// x = r * cos(PI * (u - 0.5)) * cos(2PI * v)
// y = r * cos(PI * (u - 0.5)) * sin(2PI * v)
// z = r * sin(PI * (u - 0.5))
class Sphere : public GeometryObject, public AGZ::Uncopiable
{
    Real radius_;

public:

    explicit Sphere(Real radius);

    bool HasIntersection(const Ray &ray) const override;

    Option<GeometryIntersection> EvalIntersection(const Ray &ray) const override;

    Real GetRadius() const { return radius_; }
};

AGZ_NS_END(Atrc)
