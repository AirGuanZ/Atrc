#include <agz/tracer/core/geometry.h>

AGZ_TRACER_BEGIN

class TransformWrapper : public Geometry
{
    RC<const Geometry> internal_;

    FTransform3 local_to_world_;
    real scale_ratio_ = 1;

    AABB world_bound_;

    void init_transform(const FTransform3 &local_to_world)
    {
        local_to_world_ = local_to_world;
        scale_ratio_ = local_to_world_.apply_to_vector({ 0, 0, 1 }).length();

        const auto [L, H] = internal_->world_bound();

        world_bound_ = AABB();
        world_bound_ |= local_to_world_.apply_to_point({ L.x, L.y, L.z });
        world_bound_ |= local_to_world_.apply_to_point({ L.x, L.y, H.z });
        world_bound_ |= local_to_world_.apply_to_point({ L.x, H.y, L.z });
        world_bound_ |= local_to_world_.apply_to_point({ L.x, H.y, H.z });
        world_bound_ |= local_to_world_.apply_to_point({ H.x, L.y, L.z });
        world_bound_ |= local_to_world_.apply_to_point({ H.x, L.y, H.z });
        world_bound_ |= local_to_world_.apply_to_point({ H.x, H.y, L.z });
        world_bound_ |= local_to_world_.apply_to_point({ H.x, H.y, H.z });
    }

public:

    TransformWrapper(
        RC<const Geometry> internal, const FTransform3 &local_to_world)
        : internal_(std::move(internal))
    {
        init_transform(local_to_world);
    }

    bool has_intersection(const Ray &r) const noexcept override
    {
        const Ray local_r(
            local_to_world_.apply_inverse_to_point(r.o),
            local_to_world_.apply_inverse_to_vector(r.d),
           r.t_min, r.t_max);

        return internal_->has_intersection(local_r);
    }

    bool closest_intersection(
        const Ray &r, GeometryIntersection *inct) const noexcept override
    {
        const Ray local_r(
            local_to_world_.apply_inverse_to_point(r.o),
            local_to_world_.apply_inverse_to_vector(r.d),
            r.t_min, r.t_max);

        if(!internal_->closest_intersection(local_r, inct))
            return false;

        inct->pos            = local_to_world_.apply_to_point(inct->pos);
        inct->geometry_coord = local_to_world_.apply_to_coord(inct->geometry_coord);
        inct->user_coord     = local_to_world_.apply_to_coord(inct->user_coord);
        inct->wr             = -r.d;

        return true;
    }

    AABB world_bound() const noexcept override
    {
        return world_bound_;
    }

    real surface_area() const noexcept override
    {
        return internal_->surface_area() * scale_ratio_ * scale_ratio_;
    }

    SurfacePoint sample(real *pdf, const Sample3 &sam) const noexcept override
    {
        SurfacePoint spt = internal_->sample(pdf, sam);

        spt.pos            = local_to_world_.apply_to_point(spt.pos);
        spt.geometry_coord = local_to_world_.apply_to_coord(spt.geometry_coord);
        spt.user_coord     = local_to_world_.apply_to_coord(spt.user_coord);

        *pdf /= scale_ratio_ * scale_ratio_;

        return spt;
    }

    SurfacePoint sample(
        const FVec3 &ref, real *pdf, const Sample3 &sam) const noexcept override
    {
        SurfacePoint spt = internal_->sample(
            local_to_world_.apply_inverse_to_point(ref), pdf, sam);

        spt.pos            = local_to_world_.apply_to_point(spt.pos);
        spt.geometry_coord = local_to_world_.apply_to_coord(spt.geometry_coord);
        spt.user_coord     = local_to_world_.apply_to_coord(spt.user_coord);

        *pdf /= scale_ratio_ * scale_ratio_;

        return spt;
    }

    real pdf(const FVec3 &pos) const noexcept override
    {
        return internal_->pdf(local_to_world_.apply_inverse_to_point(pos))
             / (scale_ratio_ * scale_ratio_);
    }

    real pdf(const FVec3 &ref, const FVec3 &pos) const noexcept override
    {
        return internal_->pdf(
            local_to_world_.apply_inverse_to_point(ref),
            local_to_world_.apply_inverse_to_point(pos))
            / (scale_ratio_ * scale_ratio_);
    }
};

RC<Geometry> create_transform_wrapper(
    RC<const Geometry> internal, const FTransform3 &local_to_world)
{
    return newRC<TransformWrapper>(std::move(internal), local_to_world);
}

AGZ_TRACER_END
