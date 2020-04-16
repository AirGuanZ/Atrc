#pragma once

#include <agz/tracer/common.h>

AGZ_TRACER_BEGIN

/**
 * @brief point on geometry object
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
            return pos + geometry_coord.z * EPS();
        return pos - geometry_coord.z * EPS();
    }
};

/**
 * @brief point in participating medium
 */
struct MediumPoint
{
    Vec3 pos;
};

/**
 * @brief intersection between ray and geometry object
 */
struct GeometryIntersection : SurfacePoint
{
    real t = -1;
    Vec3 wr;
};

/**
 * @brief intersection between ray and entity
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
 * @brief scattering point in participating medium
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
 * @brief shading information at an entity intersection
 */
struct ShadingPoint
{
    const BSDF *bsdf = nullptr;

    Vec3 shading_normal;

    // optional bssrdf
    const BSSRDF *bssrdf = nullptr;
};

AGZ_TRACER_END
