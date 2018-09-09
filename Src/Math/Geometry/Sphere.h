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

    // 设p = o + td, 球心为c，半径为r，则交点满足：
    //      |o + dt - c|^2 = r^2
    // 令o' = o - c，则：
    //      |d|^2 * t^2 - 2 * dot(d, o') * t + |o'|^2 = r^2
    bool HasIntersection(const Ray &ray) const override
    {
        Vec3r origin_ = ray.origin - centre_;
        float A       = LengthSquare(ray.direction);
        float B       = -Real(2.0) * Dot(ray.direction, origin_);
        float C       = radius_ * radius_ - LengthSquare(origin_);
        return B * B >= Real(4.0) * A * C;
    }

    Option<Intersection> EvalIntersection(const Ray &ray) const override
    {
        // TODO
        return None;
    }
};

AGZ_NS_END(Atrc)
