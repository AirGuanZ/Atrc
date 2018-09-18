#pragma once

#include <optional>

#include "Differential.h"
#include "Ray.h"

AGZ_NS_BEG(Atrc)

struct Intersection
{
    Real t;
    SurfaceLocal inct;
};

class GeometryObject
{
public:

    virtual ~GeometryObject() = default;

    virtual bool HasIntersection(const Ray &ray) const
    {
        return EvalIntersection(ray).has_value();
    }

    virtual Option<Intersection> EvalIntersection(const Ray &ray) const = 0;
};

AGZ_NS_END(Atrc)
