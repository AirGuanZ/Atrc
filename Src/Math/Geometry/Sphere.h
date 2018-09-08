#pragma once

#include "../../Common.h"
#include "../AGZMath.h"
#include "../Geometry.h"

AGZ_NS_BEG(Atrc)

class Sphere : public GeometryObjectWithTransform
{
    Vec3r centre_;
    Real radius_;

public:

    Sphere(const Transform *local2World,
           const Vec3r &centre, Real radius)
        : GeometryObjectWithTransform(local2World),
          centre_(centre), radius_(radius)
    {

    }

    const Vec3r &GetCentre() const
    {
        return centre_;
    }

    Real GetRadius() const
    {
        return radius_;
    }

    Option<Intersection> EvalIntersection(const Ray &ray) const override
    {
        // TODO
        return None;
    }
};

AGZ_NS_END(Atrc)
