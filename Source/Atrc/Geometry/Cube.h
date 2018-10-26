#pragma once

#include <Atrc/Core/Core.h>

AGZ_NS_BEG(Atrc)

class Cube : public Geometry
{
    Real halfSideLen_;
    Real surfaceArea_;

public:

    explicit Cube(const Transform &local2World, Real sideLen);

    bool HasIntersection(const Ray &r) const override;

    bool FindIntersection(const Ray &_r, SurfacePoint *sp) const override;

    Real SurfaceArea() const override;

    AABB LocalBound() const override;

    GeometrySampleResult Sample() const override;

    Real SamplePDF(const Vec3 &pos) const override;
};

AGZ_NS_END(Atrc)
