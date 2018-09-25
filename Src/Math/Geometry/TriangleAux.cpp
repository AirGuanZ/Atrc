#include "TriangleAux.h"

AGZ_NS_BEG(Atrc::TriangleAux)

bool HasIntersection(const Ray &r, const Vec3r &A, const Vec3r &B, const Vec3r &C)
{
    Vec3r B_A = B - A, C_A = C - A, _d = -r.direction;

    Real detD = Mat3r::FromCols(B_A, C_A, _d).Determinant();
    if(ApproxEq(RealT(detD), RealT(Real(0.0))))
        return false;
    Real invDetD = Real(1) / detD;

    Vec3r o_A = r.origin - A;
    Real alpha = Mat3r::FromCols(o_A, C_A, _d).Determinant() * invDetD;
    Real beta  = Mat3r::FromCols(B_A, o_A, _d).Determinant() * invDetD;
    Real t     = Mat3r::FromCols(B_A, C_A, o_A).Determinant() * invDetD;

    Real coefA = Real(1) - alpha - beta;
    return r.minT <= t && t <= r.maxT &&
           coefA >= Real(0.0) && alpha >= Real(0.0) && beta >= Real(0.0);
}

// IMPROVE
// Solve equations:
//      coefA * A + coefB * B + coefC * C = o + td
//      coefA + coefB + coefC = 1
// and return Some(TriangleIntersection{ coefA, coefB, coefC })
//
// An equivalent equation is:
//     A + alpha * (B-A) + beta * (C-A) = o + td
// =>  [B-A, C-A, -d][alpha, beta, t]' = o-A
// Let D = [B-A, C-A, -d], then:
// => alpha = |o-A, C-A, -d| / |D|
// => beta  = |B-A, o-A, -d| / |D|
// => t     = |B-A, C-A, o-A| / |D|
// Finally,
//     coefA = 1 - alpha - beta
//     coefB = alpha
//     coefC = beta
Option<TriangleIntersection> EvalIntersection(
    const Ray &r, const Vec3r &A, const Vec3r &B, const Vec3r &C)
{
    Vec3r B_A = B - A, C_A = C - A, _d = -r.direction;

    Real detD = Mat3r::FromCols(B_A, C_A, _d).Determinant();
    if(ApproxEq(RealT(detD), RealT(Real(0.0))))
        return None;
    Real invDetD = Real(1) / detD;

    Vec3r o_A = r.origin - A;
    Real alpha = Mat3r::FromCols(o_A, C_A, _d).Determinant() * invDetD;
    Real beta  = Mat3r::FromCols(B_A, o_A, _d).Determinant() * invDetD;
    Real t     = Mat3r::FromCols(B_A, C_A, o_A).Determinant() * invDetD;

    Real coefA = Real(1) - alpha - beta;
    if(t < r.minT || t > r.maxT ||
       alpha < Real(0.0) || beta < Real(0.0) || coefA < Real(0.0))
        return None;

    return TriangleIntersection{ coefA, alpha, beta, t };
}

// For any p on triangle ABC with parameter (u, v),
//     p = A + (u - uA) * dpdu + (v - vA) * dpdv
// which implies:
//     B = A + (uB - uA) * dpdu + (vB - vA) * dpdv
//     C = A + (uC - uA) * dpdu + (vC - vA) * dpdv
// Thus:
//     (vC-vA)(uB-uA) * dpdu + (vC-vA)(vB-vA) * dpdv = (vC-vA)(B-A)
//     (vB-vA)(uC-uA) * dpdu + (vB-vA)(vC-vA) * dpdv = (vB-vA)(C-A)
//     (uC-uA)(uB-uA) * dpdu + (uC-uA)(vB-vA) * dpdv = (uC-uA)(B-A)
//     (uB-uA)(uC-uA) * dpdu + (uB-uA)(vC-vA) * dpdv = (uB-uA)(C-A)
// =>  dpdu = ((vC-vA)(B-A) - (vB-vA)(C-A)) / ((vC-vA)(uB-uA) - (vB-vA)(uC-uA))
//     dpdv = ((uB-uA)(C-A) - (uC-uA)(B-A)) / ((uB-uA)(vC-vA) - (uC-uA)(vB-vA))
TriangleSurfaceLocal EvalSurfaceLocal(
    const Vec3r &A, const Vec3r &B, const Vec3r &C,
    const Vec2r &uvA, const Vec2r &uvB, const Vec2r &uvC)
{
    Real dem = (uvC.v - uvA.v) * (uvB.u - uvA.u) - (uvB.v - uvA.v) * (uvC.u * uvA.u);

    if(RealT(dem).ApproxEq(Real(0)))
    {
        Vec3r dpdu = C - A;
        Vec3r dpdv = Cross(Cross(dpdu, B - A), dpdu);
        return { dpdu.Normalize(), dpdv.Normalize() };
    }

    Real invDem = Real(1) / dem;
    Vec3r dpdu = ((uvC.v - uvA.v) * (B - A) - (uvB.v - uvA.v) * (C - A)) * invDem;
    Vec3r dpdv = ((uvB.u - uvA.u) * (C - A) - (uvC.u - uvA.u) * (B - A)) * invDem;
    return { dpdu, dpdv };
}

AGZ_NS_END(Atrc::TriangleAux)
