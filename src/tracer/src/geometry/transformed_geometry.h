#pragma once

#include <agz/tracer/core/geometry.h>

AGZ_TRACER_BEGIN

class TransformedGeometry : public Geometry
{
protected:

    void init_transform(const Config &params);

    void update_ratio() noexcept;

    Ray to_local(const Ray &world_ray) const noexcept;

    void to_world(SurfacePoint *spt) const noexcept;

    void to_world(GeometryIntersection *inct) const noexcept;

    AABB to_world(const AABB &local_aabb) const noexcept;

    Transform3 local_to_world_;
    real local_to_world_ratio_ = 1;

    /**
     * @brief 构造一个transform，将local_bound的中心transform到原点，且变换后的bound最长的一条轴对齐边长为1
     */
    static Transform3 pretransform(const AABB &local_bound) noexcept;

public:

    using Geometry::Geometry;
};

inline void TransformedGeometry::init_transform(const Config &params)
{
    AGZ_HIERARCHY_TRY

    local_to_world_ = params.child_transform3("transform");
    local_to_world_ratio_ = local_to_world_.apply_to_vector({ 1, 0, 0 }).length();

    AGZ_HIERARCHY_WRAP("in initializing transformed geometry")
}

inline void TransformedGeometry::update_ratio() noexcept
{
    local_to_world_ratio_ = local_to_world_.apply_to_vector({ 1, 0, 0 }).length();
}

inline Ray TransformedGeometry::to_local(const Ray &world_ray) const noexcept
{
    Vec3 local_o = local_to_world_.apply_inverse_to_point(world_ray.o);
    Vec3 local_d = local_to_world_.apply_inverse_to_vector(world_ray.d);
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
    auto [low, high] = local_aabb;

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

inline Transform3 TransformedGeometry::pretransform(const AABB &local_bound) noexcept
{
    // 先挪到原点

    Vec3 centre = real(0.5) * (local_bound.low + local_bound.high);
    Transform3 translate = Transform3::translate(-centre);

    // 找出最长的一条边

    Vec3 delta = local_bound.high - local_bound.low;
    real max_l = -1;
    for(int i = 0; i < 3; ++i)
        max_l = (std::max)(max_l, delta[i]);
    
    // 将此边放缩到1

    Transform3 scale = Transform3::scale(Vec3(1 / max_l));

    return scale * translate;
}

AGZ_TRACER_END
