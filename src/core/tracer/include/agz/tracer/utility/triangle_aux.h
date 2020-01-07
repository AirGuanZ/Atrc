#pragma once

#include <cassert>

#include <agz/tracer/common.h>

AGZ_TRACER_BEGIN

inline bool has_intersection_with_triangle(
    const Ray &r, const Vec3 &A, const Vec3 &B_A, const Vec3 &C_A) noexcept
{
    const Vec3 s1 = cross(r.d, C_A);
    const real div = dot(s1, B_A);
    if(!div)
        return false;
    const real inv_div = 1 / div;

    const Vec3 o_A = r.o - A;
    const real alpha = dot(o_A, s1) * inv_div;
    if(alpha < 0 || alpha > 1)
        return false;

    const Vec3 s2 = cross(o_A, B_A);
    const real beta = dot(r.d, s2) * inv_div;
    if(beta < 0 || alpha + beta > 1)
        return false;

    const real t = dot(C_A, s2) * inv_div;
    return r.between(t);
}

struct TriangleIntersectionRecord
{
    real t_ray = 0;
    Vec2 uv;
};

inline bool closest_intersection_with_triangle(
    const Ray &r, const Vec3 &A, const Vec3 &B_A, const Vec3 &C_A,
    TriangleIntersectionRecord *record) noexcept
{
    assert(record);

    const Vec3 s1 = cross(r.d, C_A);
    const real div = dot(s1, B_A);
    if(!div)
        return false;
    const real inv_div = 1 / div;

    const Vec3 o_A = r.o - A;
    const real alpha = dot(o_A, s1) * inv_div;
    if(alpha < 0)
        return false;

    const Vec3 s2 = cross(o_A, B_A);
    const real beta = dot(r.d, s2) * inv_div;
    if(beta < 0 || alpha + beta > 1)
        return false;

    const real t = dot(C_A, s2) * inv_div;

    if(!r.between(t))
        return false;

    record->t_ray = t;
    record->uv    = Vec2(alpha, beta);

    return true;
}

inline real triangle_area(const Vec3 &B_A, const Vec3 &C_A) noexcept
{
    return cross(B_A, C_A).length() / 2;
}

inline Vec3 dpdu_as_ex(
    const Vec3 &B_A, const Vec3 &C_A,
    const Vec2 &b_a, const Vec2 &c_a,
    const Vec3 &nor)
{
    const real m00 = b_a.x, m01 = b_a.y;
    const real m10 = c_a.x, m11 = c_a.y;
    const real det = m00 * m11 - m01 * m10;
    if(!det)
        return Coord::from_z(nor).x;
    const real inv_det = 1 / det;
    return (m11 * inv_det * B_A - m01 * inv_det * C_A).normalize();
}

AGZ_TRACER_END
