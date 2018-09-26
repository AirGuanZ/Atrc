#pragma once

#include "../AGZMath.h"
#include "../Geometry.h"
#include "../Ray.h"

AGZ_NS_BEG(Atrc::TriangleAux)

struct TriangleIntersection
{
    Real coefA, coefB, coefC, t;
};

struct TriangleSurfaceLocal
{
    Vec3r normal, dpdu, dpdv;
};

bool HasIntersection(const Ray &r, const Vec3r &A, const Vec3r &B_A, const Vec3r &C_A);

Option<TriangleIntersection> EvalIntersection(
    const Ray &r, const Vec3r &A, const Vec3r &B_A, const Vec3r &C_A);

TriangleSurfaceLocal EvalSurfaceLocal(
    const Vec3r &B_A, const Vec3r &C_A,
    const Vec2r &uvB_A, const Vec2r &uvC_A);

AGZ_FORCEINLINE bool HasIntersection2(
    const Ray &r, const Vec3r &A, const Vec3r &B, const Vec3r &C)
{
    return HasIntersection(r, A, B - A, C - A);
}

AGZ_FORCEINLINE Option<TriangleIntersection> EvalIntersection2(
    const Ray &r, const Vec3r &A, const Vec3r &B, const Vec3r &C)
{
    return EvalIntersection(r, A, B - A, C - A);
}

// p = A + (u - uA) * dpdu + (v - vA) * dpdv
AGZ_FORCEINLINE TriangleSurfaceLocal EvalSurfaceLocal2(
    const Vec3r &A, const Vec3r &B, const Vec3r &C,
    const Vec2r &uvA, const Vec2r &uvB, const Vec2r &uvC)

{
    return EvalSurfaceLocal(B - A, C - A, uvB - uvA, uvC - uvA);
}

AGZ_NS_END(Atrc::TriangleAux)
