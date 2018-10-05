#pragma once

#include <Atrc/Common.h>
#include <Atrc/Math/AABB.h>
#include <Atrc/Math/Ray.h>

AGZ_NS_BEG(Atrc::Geometry::Sphere)

bool HasIntersection(const Ray &r, Real radius);

bool EvalIntersection(const Ray &r, Real radius, Intersection *inct);

AGZ_NS_END(Atrc::Geometry::Sphere)

AGZ_NS_BEG(Atrc::Geometry::Triangle)

bool HasIntersection(const Vec3r &A, const Vec3r &B_A, const Vec3r &C_A, const Ray &r);

bool EvalIntersection(
    const Vec3r &A, const Vec3r &B_A, const Vec3r &C_A,
    const Ray &r, Intersection *inct);

AABB ToBoundingBox(const Vec3r &A, const Vec3r &B, const Vec3r &C);

Real SurfaceArea(const Vec3r &A, const Vec3r &B, const Vec3r &C);

AGZ_NS_END(Atrc::Geometry::Triangle)
