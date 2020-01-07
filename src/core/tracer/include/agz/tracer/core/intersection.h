#pragma once

#include <agz/tracer/common.h>

AGZ_TRACER_BEGIN

class BSDF;
class BSSRDF;
class Entity;
class Geometry;
class Material;
class Medium;

/**
 * @brief 几何体表面上的一点
 */
struct SurfacePoint
{
    Vec3 pos;
    Vec2 uv;
    Coord geometry_coord;
    Coord user_coord;

    Vec3 eps_offset(const Vec3 &dir) const noexcept
    {
        if(dot(dir, geometry_coord.z) > 0)
            return pos + geometry_coord.z * EPS;
        return pos - geometry_coord.z * EPS;
    }
};

/**
 * @brief 介质中的一点
 */
struct MediumPoint
{
    Vec3 pos;
};

/**
 * @brief 射线与几何体表面的交点
 */
struct GeometryIntersection : SurfacePoint
{
    real t = -1;
    Vec3 wr;
};

/**
 * @brief 射线与实体表面的交点
 */
struct EntityIntersection : GeometryIntersection
{
    const Entity *entity     = nullptr;
    const Material *material = nullptr;
    const Medium *medium_in  = nullptr;
    const Medium *medium_out = nullptr;

    const Medium *wr_medium() const noexcept
    {
        return dot(wr, geometry_coord.z) >= 0 ? medium_out : medium_in;
    }

    const Medium *inv_wr_medium() const noexcept
    {
        return dot(wr, geometry_coord.z) >= 0 ? medium_in : medium_out;
    }

    const Medium *medium(const Vec3 &d) const noexcept
    {
        return dot(d, geometry_coord.z) >= 0 ? medium_out : medium_in;
    }
};

/**
 * @brief 射线在介质中的散射点
 */
struct MediumScattering : MediumPoint
{
    const Medium *medium = nullptr;
    Vec3 wr;

    bool invalid() const noexcept
    {
        return !wr;
    }
};

/**
 * @brief 实体表面一点处的着色信息
 */
struct ShadingPoint
{
    const BSDF *bsdf = nullptr;
    Vec3 shading_normal;
};

AGZ_TRACER_END
