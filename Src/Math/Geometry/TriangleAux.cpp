#include "TriangleAux.h"

AGZ_NS_BEG(Atrc::TriangleAux)

bool HasIntersection(const Ray &r, const Vec3r &A, const Vec3r &B, const Vec3r &C)
{
    // TODO
    return EvalIntersection(r, A, B, C).has_value();
}

// Solve equations:
//      coefA * A + coefB * B + coefC * C = o + td
//      coefA + coefB + coefC = 1
// and return Some(TriangleGeometryIntersection{ coefA, coefB, coefC })
Option<TriangleGeometryIntersection> EvalIntersection(
    const Ray &r, const Vec3r &A, const Vec3r &B, const Vec3r &C)
{
    // TODO
    return None;
}

AGZ_NS_END(Atrc::TriangleAux)
