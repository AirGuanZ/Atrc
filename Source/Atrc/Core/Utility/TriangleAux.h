#pragma once

#include <Atrc/Core/Core/Common.h>
#include <Atrc/Core/Core/Ray.h>

namespace Atrc
{
    
inline bool HasIntersectionWithTriangle(
    const Ray &r, const Vec3 &A, const Vec3 &B_A, const Vec3 &C_A) noexcept
{
    Vec3 s1 = Cross(r.d, C_A);
    Real div = Dot(s1, B_A);
    if(!div)
        return false;
    Real invDiv = 1 / div;

    Vec3 o_A = r.o - A;
    Real alpha = Dot(o_A, s1) * invDiv;
    if(alpha < 0 || alpha > 1)
        return false;

    Vec3 s2 = Cross(o_A, B_A);
    Real beta = Dot(r.d, s2) * invDiv;
    if(beta < 0 || alpha + beta > 1)
        return false;

    Real t = Dot(C_A, s2) * invDiv;
    return r.Between(t);
}

struct TriangleIntersectionRecoed
{
    Real t;
    Vec2 uv;
};

inline bool FindIntersectionWithTriangle(
    const Ray &r, const Vec3 &A, const Vec3 &B_A, const Vec3 &C_A,
    TriangleIntersectionRecoed *record) noexcept
{
    AGZ_ASSERT(record);

    Vec3 s1 = Cross(r.d, C_A);
    Real div = Dot(s1, B_A);
    if(!div)
        return false;
    Real invDiv = 1 / div;

    Vec3 o_A = r.o - A;
    Real alpha = Dot(o_A, s1) * invDiv;
    if(alpha < 0)
        return false;

    Vec3 s2 = Cross(o_A, B_A);
    Real beta = Dot(r.d, s2) * invDiv;
    if(beta < 0 || alpha + beta > 1)
        return false;

    Real t = Dot(C_A, s2) * invDiv;

    if(!r.Between(t))
        return false;

    record->t = t;
    record->uv = Vec2(alpha, beta);

    return true;
}

inline Real GetTriangleArea(const Vec3 &B_A, const Vec3 &C_A) noexcept
{
    return Cross(B_A, C_A).Length() / 2;
}

inline Vec3 ComputeDpduAsEx(
    const Vec3 &B_A, const Vec3 &C_A,
    const Vec2 &b_a, const Vec2 &c_a,
    const Vec3 &nor)
{
    Real m00 = b_a.u, m01 = b_a.v;
    Real m10 = c_a.u, m11 = c_a.v;
    Real det = m00 * m11 - m01 * m10;
    if(!det)
        return CoordSystem::FromEz(nor).ex;
    Real invDet = 1 / det;
    return (m11 * invDet * B_A - m01 * invDet * C_A).Normalize();
}

} // namespace Atrc
