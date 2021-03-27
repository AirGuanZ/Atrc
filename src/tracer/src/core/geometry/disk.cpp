#include <agz-utils/misc.h>
#include "./transformed_geometry.h"

AGZ_TRACER_BEGIN

class Disk : public TransformedGeometry
{
    real radius_ = 1;
    real radius2_ = 1;

    SurfacePoint to_local_surface_point(
        const FVec3 &pos, real radius) const noexcept
    {
        const real phi = local_angle::phi(pos);
        const real u = phi / (2 * PI_r);
        const real v = radius / radius_;

        const FVec3 coord_x = FVec3(pos.x, pos.y, 0).normalize();
        const FVec3 coord_z = FVec3(0, 0, 1);
        const FVec3 coord_y = cross(coord_z, coord_x);

        SurfacePoint ret;
        ret.pos            = pos;
        ret.uv             = Vec2(u, v);
        ret.geometry_coord = FCoord(coord_x, coord_y, coord_z);
        ret.user_coord     = ret.geometry_coord;

        return ret;
    }

public:

    Disk(real radius, const FTransform3 &transform)
    {
        AGZ_HIERARCHY_TRY

        init_transform(transform);

        if(radius <= 0)
            throw ObjectConstructionException(
                "invalid radius value: " + std::to_string(radius));
        radius_ = radius;
        radius2_ = radius * radius;

        AGZ_HIERARCHY_WRAP("in initializing disk geometry object")
    }

    bool has_intersection(const Ray &r) const noexcept override
    {
        const Ray local_r = to_local(r);
        
        const real t = -local_r.o.z / local_r.d.z;
        if(std::isinf(t) || !local_r.between(t))
            return false;

        const FVec3 pnt = local_r.at(t);
        const real radius2 = Vec2(pnt.x, pnt.y).length_square();
        return radius2 <= radius2_;
    }

    bool closest_intersection(
        const Ray &r, GeometryIntersection *inct) const noexcept override
    {
        const Ray local_r = to_local(r);

        const real t_val = -local_r.o.z / local_r.d.z;
        if(std::isinf(t_val) || !local_r.between(t_val))
            return false;

        const FVec3 pos = local_r.at(t_val);
        const real radius = Vec2(pos.x, pos.y).length();
        if(radius > radius_)
            return false;

        *static_cast<SurfacePoint*>(inct) = to_local_surface_point(pos, radius);
        inct->wr = -local_r.d;
        inct->t = t_val;

        to_world(inct);
        return true;
    }

    AABB world_bound() const noexcept override
    {
        const real vertical_size = (std::min)(real(0.1) * radius_, real(0.01));
        const AABB local_bound = {
            FVec3(-radius_, -radius_, -vertical_size),
            FVec3(radius_, radius_, vertical_size)
        };
        return to_world(local_bound);
    }

    real surface_area() const noexcept override
    {
        return local_to_world_ratio_ * local_to_world_ratio_ * PI_r * radius2_;
    }

    SurfacePoint sample(real *pdf, const Sample3 &sam) const noexcept override
    {
        const Vec2 pos = radius_ * math::distribution
                                    ::uniform_on_unit_disk(sam.u, sam.v);
        *pdf = 1 / surface_area();

        SurfacePoint ret = to_local_surface_point(
                                FVec3(pos.x, pos.y, 0), pos.length());
        to_world(&ret);

        return ret;
    }

    real pdf(const FVec3 &) const noexcept override
    {
        return 1 / surface_area();
    }

    SurfacePoint sample(
        const FVec3&, real *pdf, const Sample3 &sam) const noexcept override
    {
        return sample(pdf, sam);
    }

    real pdf(const FVec3&, const FVec3 &sample) const noexcept override
    {
        return pdf(sample);
    }
};

RC<Geometry> create_disk(
    real radius, const FTransform3 &local_to_world)
{
    return newRC<Disk>(radius, local_to_world);
}

AGZ_TRACER_END
