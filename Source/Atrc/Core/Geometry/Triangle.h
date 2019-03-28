#pragma once

#include <Atrc/Core/Core/Geometry.h>

namespace Atrc
{

class Triangle : public Geometry
{
    Vec3 A, B_A, C_A;
    Vec2 tA, tB_tA, tC_tA;
    Vec3 ex, nor;

public:

    Triangle(
        const Transform &local2World, const Vec3 &A, const Vec3 &B, const Vec3 &C,
        const Vec2 &tA, const Vec2 &tB, const Vec2 &tC) noexcept;

    bool HasIntersection(const Ray &r) const noexcept override;

    bool FindIntersection(const Ray &r, GeometryIntersection *inct) const noexcept override;

    Real GetSurfaceArea() const noexcept override;

    AABB GetLocalBound() const noexcept override;

    AABB GetWorldBound() const noexcept override;

    SampleResult Sample(const Vec3 &sample) const noexcept override;

    Real SamplePDF(const Vec3 &pos) const noexcept override;
};

} // namespace Atrc
