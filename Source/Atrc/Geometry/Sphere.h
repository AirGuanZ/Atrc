#pragma once

#include <Atrc/Core/Geometry.h>

AGZ_NS_BEG(Atrc)

class Sphere : public Geometry
{
    Real radius_;

public:

    Sphere(const Transform &local2World, Real radius)
        : Geometry(local2World), radius_(radius)
    {
        AGZ_ASSERT(radius_ > 0.0);
    }

    bool HasIntersection(const Ray &_r) const override;

    bool FindIntersection(const Ray &_r, SurfacePoint *sp) const override;

    Real SurfaceArea() const override;

    AABB LocalBound() const override;

    GeometrySampleResult Sample() const override;

    Real SamplePDF(const SurfacePoint &sp) const override;
};

AGZ_NS_END(Atrc)
