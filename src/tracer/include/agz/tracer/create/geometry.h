#pragma once

#include <agz/tracer/core/geometry.h>
#include <agz/utility/mesh.h>

AGZ_TRACER_BEGIN

RC<Geometry> create_disk(
    real radius, const FTransform3 &local_to_world);

RC<Geometry> create_double_sided(
    RC<const Geometry> internal);

RC<Geometry> create_quad(
    const FVec3 &a, const FVec3 &b, const FVec3 &c, const FVec3 &d,
    const Vec2 &t_a, const Vec2 &t_b, const Vec2 &t_c, const Vec2 &t_d,
    const FTransform3 &local_to_world);

RC<Geometry> create_sphere(
    real radius, const FTransform3 &local_to_world);

RC<Geometry> create_transform_wrapper(
    RC<const Geometry> internal, const FTransform3 &local_to_world);

RC<Geometry> create_triangle(
    const FVec3 &a, const FVec3 &b, const FVec3 &c,
    const Vec2 &t_a, const Vec2 &t_b, const Vec2 &t_c,
    const FTransform3 &local_to_world);

RC<Geometry> create_triangle_bvh(
    std::vector<mesh::triangle_t> build_triangles,
    const FTransform3 &local_to_world);

#ifdef USE_EMBREE

RC<Geometry> create_triangle_bvh_embree(
    std::vector<mesh::triangle_t> build_triangles,
    const FTransform3 &local_to_world);

#endif

RC<Geometry> create_triangle_bvh_noembree(
    std::vector<mesh::triangle_t> build_triangles,
    const FTransform3 &local_to_world);

AGZ_TRACER_END
