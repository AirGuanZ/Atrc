#pragma once

#include <Atrc/Lib/Core/Geometry.h>

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

    bool HasIntersection(const Ray &r) const noexcept override;

    bool FindIntersection(const Ray &r, GeometryIntersection *sp) const noexcept override;

    Real SurfaceArea() const noexcept override;

    CoordSystem GetShadingCoordSys(const GeometryIntersection &inct) const noexcept override;

    Vec2 GetShadingUV(const Intersection &inct) const noexcept override;

    AABB GetLocalBound() const noexcept override;

    SampleResult Sample() const noexcept override;

    Real SamplePDF(const Vec3 &pos) const noexcept override;
};

} // namespace Atrc
