#pragma once

#include <optional>
#include <Utils/Math.h>

#include "Differential.h"
#include "Ray.h"
#include "Transform.h"

AGZ_NS_BEG(Atrc)

struct Intersection
{
    Real t;
    SurfaceLocal inct;
};

class GeometryObject
{
protected:

    virtual bool HasIntersectionImpl(
        const Ray &ray, Real minT, Real maxT) const
    {
        return EvalIntersectionImpl(ray, minT, maxT).has_value();
    }

    virtual std::optional<Intersection> EvalIntersectionImpl(
        const Ray &ray, Real minT, Real maxT) const = 0;

public:

    virtual ~GeometryObject() = default;

    bool HasIntersection(
        const Ray &ray,
        Real minT = Real(0.0),
        Real maxT = FP<Real>::Max()) const
    {
        return HasIntersectionImpl(ray, minT, maxT);
    }

    std::optional<Intersection> EvalIntersection(
        const Ray &ray,
        Real minT = Real(0.0),
        Real maxT = FP<Real>::Max()) const
    {
        return EvalIntersectionImpl(ray, minT, maxT);
    }
};

AGZ_NS_END(Atrc)
