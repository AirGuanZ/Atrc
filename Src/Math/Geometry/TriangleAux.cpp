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
    Real beta = Mat3r::FromCols(B_A, o_A, _d).Determinant() * invDetD;

    Real coefA = Real(1) - alpha - beta;
    return coefA >= Real(0.0) && alpha >= Real(0.0) && beta >= Real(0.0);
}

// Solve equations:
//      coefA * A + coefB * B + coefC * C = o + td
//      coefA + coefB + coefC = 1
// and return Some(TriangleGeometryIntersection{ coefA, coefB, coefC })
//      An equivalent equation is:
//          A + alpha * (B-A) + beta * (C-A) = o + td
//      =>  [B-A, C-A, -d][alpha, beta, t]' = o-A
//      Let D = [B-A, C-A, -d]
//      => alpha = |o-A, C-A, -d|/|D|
//      => beta  = |B-A, o-A, -d|/|D|
//      => t     = |B-A, C-A, o-A|/|D|
//      And:
//          coefA = 1 - alpha - beta
//          coefB = alpha
//          coefC = beta
Option<TriangleGeometryIntersection> EvalIntersection(
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

    Real coefA = Real(1) - alpha - beta;
    if(alpha < Real(0.0) || beta < Real(0.0) || coefA < Real(0.0))
        return None;

    return TriangleGeometryIntersection{ coefA, alpha, beta };
}

AGZ_NS_END(Atrc::TriangleAux)
