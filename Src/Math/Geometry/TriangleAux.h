#pragma once

#include "../AGZMath.h"
#include "../Geometry.h"
#include "../Ray.h"

AGZ_NS_BEG(Atrc::TriangleAux)

bool HasIntersection(const Ray &r, const Vec3r &A, const Vec3r &B, const Vec3r &C);

struct TriangleIntersection
{
    Real coefA, coefB, coefC, t;
};

Option<TriangleIntersection> EvalIntersection(
    const Ray &r, const Vec3r &A, const Vec3r &B, const Vec3r &C);

struct TriangleSurfaceLocal
{
    Vec3r dpdu;
    Vec3r dpdv;
};

// p = A + u * dpdu + v * dpdv
TriangleSurfaceLocal EvalSurfaceLocal(
    const Vec3r &A,   const Vec3r &B,   const Vec3r &C,
    const Vec2r &uvA, const Vec2r &uvB, const Vec2r &uvC);

AGZ_NS_END(Atrc::TriangleAux)
