#include "TriangleAux.h"

AGZ_NS_BEG(Atrc::TriangleAux)

bool HasIntersection(const Ray &r, const Vec3r &A, const Vec3r &B_A, const Vec3r &C_A)
{
    Vec3r _d = -r.direction;

    Real detD = Mat3r::FromCols(B_A, C_A, _d).Determinant();
    if(ApproxEq(RealT(detD), RealT(Real(0.0))))
        return false;
    Real invDetD = Real(1) / detD;

    Vec3r o_A = r.origin - A;
    Real alpha = Mat3r::FromCols(o_A, C_A, _d).Determinant() * invDetD;
    Real beta = Mat3r::FromCols(B_A, o_A, _d).Determinant() * invDetD;
    Real t = Mat3r::FromCols(B_A, C_A, o_A).Determinant() * invDetD;

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
    const Ray &r, const Vec3r &A, const Vec3r &B_A, const Vec3r &C_A)
{
    Vec3r _d = -r.direction;

    Real detD = Mat3r::FromCols(B_A, C_A, _d).Determinant();
    if(ApproxEq(RealT(detD), RealT(Real(0.0))))
        return None;
    Real invDetD = Real(1) / detD;

    Vec3r o_A = r.origin - A;
    Real alpha = Mat3r::FromCols(o_A, C_A, _d).Determinant() * invDetD;
    Real beta = Mat3r::FromCols(B_A, o_A, _d).Determinant() * invDetD;
    Real t = Mat3r::FromCols(B_A, C_A, o_A).Determinant() * invDetD;

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
    const Vec3r &B_A, const Vec3r &C_A,
    const Vec2r &uvB_A, const Vec2r &uvC_A)
{
    Real dem = uvC_A.v * uvB_A.u - uvB_A.v * uvC_A.u;

    if(RealT(dem).ApproxEq(Real(0)))
    {
        Vec3r norm = Cross(C_A, B_A);
        Vec3r dpdv = Cross(norm, C_A);
        return { norm.Normalize(), C_A.Normalize(), dpdv.Normalize() };
    }

    Real invDem = Real(1) / dem;
    Vec3r dpdu = (uvC_A.v * B_A - uvB_A.v * C_A) * invDem;
    Vec3r dpdv = (uvB_A.u * C_A - uvC_A.u * B_A) * invDem;
    return { Cross(C_A, B_A).Normalize(), dpdu, dpdv };
}

AGZ_NS_END(Atrc::TriangleAux)
