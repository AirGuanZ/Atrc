#pragma once

#include <Atrc/Common.h>
#include <Atrc/Entity/Entity.h>
#include <Atrc/Light/GeoDiffuseLight.h>
#include <Atrc/Math/Math.h>

AGZ_NS_BEG(Atrc)

class Sphere
    : ATRC_IMPLEMENTS Entity
{
protected:

    Real radius_;

public:

    explicit Sphere(Real radius);

    bool HasIntersection(const Ray &r) const override;

    bool EvalIntersection(const Ray &r, Intersection *inct) const override;

    AABB GetBoundingBox() const override;

    Real SurfaceArea() const override;

    Option<GeometrySurfaceSample> SampleSurface() const;
};

AGZ_NS_END(Atrc)
