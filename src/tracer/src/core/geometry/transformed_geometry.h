#pragma once

#include <agz/tracer/core/geometry.h>

AGZ_TRACER_BEGIN

class TransformedGeometry : public Geometry
{
protected:

    void init_transform(const FTransform3 &local_to_world);

    void update_ratio() noexcept;

    Ray to_local(const Ray &world_ray) const noexcept;

    void to_world(SurfacePoint *spt) const noexcept;

    void to_world(GeometryIntersection *inct) const noexcept;

    AABB to_world(const AABB &local_aabb) const noexcept;

    FTransform3 local_to_world_;
    real local_to_world_ratio_ = 1;

public:

    using Geometry::Geometry;
};

inline void TransformedGeometry::init_transform(const FTransform3 &local_to_world)
{
    AGZ_HIERARCHY_TRY

    local_to_world_ = local_to_world;
    local_to_world_ratio_ = local_to_world_.apply_to_vector({ 1, 0, 0 }).length();

    AGZ_HIERARCHY_WRAP("in initializing transformed geometry")
}

inline void TransformedGeometry::update_ratio() noexcept
{
    local_to_world_ratio_ = local_to_world_.apply_to_vector({ 1, 0, 0 }).length();
}

inline Ray TransformedGeometry::to_local(const Ray &world_ray) const noexcept
{
    const FVec3 local_o = local_to_world_.apply_inverse_to_point(world_ray.o);
    const FVec3 local_d = local_to_world_.apply_inverse_to_vector(world_ray.d);
    return Ray(local_o, local_d, world_ray.t_min, world_ray.t_max);
}

inline void TransformedGeometry::to_world(SurfacePoint *spt) const noexcept
{
    assert(spt);
    spt->pos                = local_to_world_.apply_to_point(spt->pos);
    spt->geometry_coord     = local_to_world_.apply_to_coord(spt->geometry_coord);
    spt->user_coord         = local_to_world_.apply_to_coord(spt->user_coord);
}

inline void TransformedGeometry::to_world(GeometryIntersection *inct) const noexcept
{
    to_world(static_cast<SurfacePoint*>(inct));
    inct->wr = local_to_world_.apply_to_vector(inct->wr);
}

inline AABB TransformedGeometry::to_world(const AABB &local_aabb) const noexcept
{
    const auto [low, high] = local_aabb;

    AABB ret;
    ret |= local_to_world_.apply_to_point(low);
    ret |= local_to_world_.apply_to_point({ high.x, low.y,  low.z });
    ret |= local_to_world_.apply_to_point({ low.x,  high.y, low.z });
    ret |= local_to_world_.apply_to_point({ low.x,  low.y,  high.z });
    ret |= local_to_world_.apply_to_point({ low.x,  high.y, high.z });
    ret |= local_to_world_.apply_to_point({ high.x, low.y,  high.z });
    ret |= local_to_world_.apply_to_point({ high.x, high.y, low.z });
    ret |= local_to_world_.apply_to_point(high);

    return ret;
}

AGZ_TRACER_END
