#pragma once

#include <Atrc/Core/Core.h>

AGZ_NS_BEG(Atrc)

class Triangle : public Geometry
{
    Vec3 A, B_A, C_A;
    Vec2 tA, tB_tA, tC_tA;
    Vec3 ex, ez;

    Real surfaceArea_;

public:

    Triangle(
        const Transform &local2World,
        const Vec3 &A, const Vec3 &B, const Vec3 &C,
        const Vec2 &tA, const Vec2 &tB, const Vec2 &tC);
    
    bool HasIntersection(const Ray &_r) const override;

    bool FindIntersection(const Ray &_r, SurfacePoint *sp) const override;

    Real SurfaceArea() const override;

    AABB LocalBound() const override;

    GeometrySampleResult Sample() const override;

    Real SamplePDF(const Vec3 &pos) const override;
};

AGZ_NS_END(Atrc)
