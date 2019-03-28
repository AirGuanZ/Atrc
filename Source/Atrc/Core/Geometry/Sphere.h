#pragma once

#include <Atrc/Core/Core/Geometry.h>

namespace Atrc
{

class Sphere : public Geometry
{
    Real radius_;

public:

    Sphere(const Transform &local2World, Real radius)
        : Geometry(local2World), radius_(radius)
    {
        AGZ_ASSERT(radius_ > 0.0);
    }

    bool HasIntersection(const Ray &_r) const noexcept override;

    bool FindIntersection(const Ray &_r, GeometryIntersection *sp) const noexcept override;

    Real GetSurfaceArea() const noexcept override;

    AABB GetLocalBound() const noexcept override;

    SampleResult Sample(const Vec3 &sample) const noexcept override;

    Real SamplePDF(const Vec3 &pos) const noexcept override;

    SampleResult Sample(const Vec3 &ref, const Vec3 &sample) const noexcept override;

    Real SamplePDF(const Vec3 &pos, const Vec3 &ref) const noexcept override;
};

} // namespace Atrc
