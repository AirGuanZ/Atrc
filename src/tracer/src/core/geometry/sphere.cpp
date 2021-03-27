#include <agz/tracer/utility/sphere_aux.h>
#include <agz-utils/misc.h>

#include "./transformed_geometry.h"

AGZ_TRACER_BEGIN

class Sphere : public TransformedGeometry
{
    real radius_;
    real world_radius_;

public:

    Sphere(real radius, const FTransform3 &local_to_world)
    {
        AGZ_HIERARCHY_TRY

        init_transform(local_to_world);
        radius_ = radius;
        if(radius_ <= 0)
            throw ObjectConstructionException(
                "invalid sphere radius: " + std::to_string(radius_));
        world_radius_ = local_to_world_ratio_ * radius_;

        AGZ_HIERARCHY_WRAP("in initializing sphere")
    }

    bool has_intersection(const Ray &r) const noexcept override
    {
        const Ray local_r = to_local(r);
        return sphere::has_intersection(local_r, radius_);
    }

    bool closest_intersection(
        const Ray &r, GeometryIntersection *inct) const noexcept override
    {
        const Ray local_r = to_local(r);

        real t;
        if(!sphere::closest_intersection(local_r, &t, radius_))
            return false;

        const FVec3 pos = local_r.at(t);

        Vec2 geometry_uv(UNINIT);
        FCoord geometry_coord(UNINIT);
        sphere::local_geometry_uv_and_coord(
            pos, &geometry_uv, &geometry_coord, radius_);

        inct->pos = pos;
        inct->geometry_coord = geometry_coord;
        inct->uv = geometry_uv;
        inct->user_coord = geometry_coord;
        inct->wr = -local_r.d;
        inct->t = t;

        to_world(inct);

        return true;
    }

    AABB world_bound() const noexcept override
    {
        const FVec3 world_origin = local_to_world_.apply_to_point(FVec3(0));
        return {
            world_origin - FVec3(world_radius_ + EPS()),
            world_origin + FVec3(world_radius_ + EPS())
        };
    }

    real surface_area() const noexcept override
    {
        return 4 * PI_r * world_radius_ * world_radius_;
    }

    SurfacePoint sample(real *pdf, const Sample3 &sam) const noexcept override
    {
        assert(pdf);

        const auto [unit_pos, unit_pdf] = math::distribution
                                            ::uniform_on_sphere(sam.u, sam.v);
        *pdf = unit_pdf / (world_radius_ * world_radius_);
        const FVec3 pos = radius_ * unit_pos;

        Vec2 geometry_uv(UNINIT); FCoord geometry_coord(UNINIT);
        sphere::local_geometry_uv_and_coord(
            pos, &geometry_uv, &geometry_coord, radius_);

        SurfacePoint spt;
        spt.pos            = pos;
        spt.geometry_coord = geometry_coord;
        spt.uv             = geometry_uv;
        spt.user_coord     = geometry_coord;
        to_world(&spt);

        return spt;
    }

    SurfacePoint sample(
        const FVec3 &ref, real *pdf, const Sample3 &sam) const noexcept override
    {
        const FVec3 local_ref = local_to_world_.apply_inverse_to_point(ref);
        const real d = local_ref.length();
        if(d <= radius_)
            return sample(pdf, sam);

        const real cos_theta = (std::min)(radius_ / d, real(1));
        const auto [dir, l_pdf] = math::distribution
                                    ::uniform_on_cone(cos_theta, sam.u, sam.v);
                                    
        const FVec3 pos = radius_ *
                         FCoord::from_z(local_ref).local_to_global(dir).normalize();

        Vec2 geometry_uv; FCoord geometry_coord;
        sphere::local_geometry_uv_and_coord(
            pos, &geometry_uv, &geometry_coord, radius_);

        SurfacePoint spt;
        spt.pos            = pos;
        spt.geometry_coord = geometry_coord;
        spt.uv             = geometry_uv;
        spt.user_coord     = geometry_coord;
        to_world(&spt);

        const real world_radius_square = world_radius_ * world_radius_;
        *pdf = l_pdf / world_radius_square;

        return spt;
    }

    real pdf(const FVec3 &sample) const noexcept override
    {
        return 1 / surface_area();
    }

    real pdf(const FVec3 &ref, const FVec3 &sample) const noexcept override
    {
        const FVec3 local_ref = local_to_world_.apply_inverse_to_point(ref);
        const real d = local_ref.length();
        if(d <= radius_)
            return pdf(sample);

        const real cos_theta = (std::min)(radius_ / d, real(1));
        const real world_radius_square = world_radius_ * world_radius_;
        
        return math::distribution::uniform_on_cone_pdf(
            cos_theta) / world_radius_square;
    }
};

RC<Geometry> create_sphere(
    real radius, const FTransform3 &local_to_world)
{
    return newRC<Sphere>(radius, local_to_world);
}

AGZ_TRACER_END
