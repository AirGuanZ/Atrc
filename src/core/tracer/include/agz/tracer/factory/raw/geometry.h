#pragma once

#include <agz/tracer/core/geometry.h>

AGZ_TRACER_BEGIN

std::shared_ptr<Geometry> create_disk(
    real radius, const Transform3 &local_to_world);

std::shared_ptr<Geometry> create_double_sided(
    std::shared_ptr<const Geometry> internal);

std::shared_ptr<Geometry> create_quad(
    const Vec3 &a, const Vec3 &b, const Vec3 &c, const Vec3 &d,
    const Vec2 &t_a, const Vec2 &t_b, const Vec2 &t_c, const Vec2 &t_d,
    const Transform3 &local_to_world);

std::shared_ptr<Geometry> create_sphere(
    real radius, const Transform3 &local_to_world);

std::shared_ptr<Geometry> create_triangle(
    const Vec3 &a, const Vec3 &b, const Vec3 &c,
    const Vec2 &t_a, const Vec2 &t_b, const Vec2 &t_c,
    const Transform3 &local_to_world);

std::shared_ptr<Geometry> create_triangle_bvh(
    const std::string &filename,
    const Transform3 &local_to_world);

#ifdef USE_EMBREE

std::shared_ptr<Geometry> create_triangle_bvh_embree(
    const std::string &filename,
    const Transform3 &local_to_world);

#endif

std::shared_ptr<Geometry> create_triangle_bvh_noembree(
    const std::string &filename,
    const Transform3 &local_to_world);

AGZ_TRACER_END
