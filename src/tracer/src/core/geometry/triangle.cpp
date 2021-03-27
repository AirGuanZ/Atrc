#include <agz/tracer/utility/triangle_aux.h>
#include <agz-utils/misc.h>

#include "./transformed_geometry.h"

AGZ_TRACER_BEGIN

class Triangle : public TransformedGeometry
{
public:

    struct Params
    {
        FVec3 a, b, c;
        Vec2 t_a, t_b, t_c;
        FTransform3 local_to_world;
    };

    explicit Triangle(const Params &params)
    {
        params_ = params;
        init_from_params(params);
    }

    bool has_intersection(const Ray &r) const noexcept override
    {
        const Ray local_r = to_local(r);
        return has_intersection_with_triangle(local_r, a_, b_a_, c_a_);
    }

    bool closest_intersection(
        const Ray &r, GeometryIntersection *inct) const noexcept override
    {
        const Ray local_r = to_local(r);
        TriangleIntersectionRecord inct_rcd;
        if(!closest_intersection_with_triangle(
            local_r, a_, b_a_, c_a_, &inct_rcd))
            return false;

        inct->pos            = local_r.at(inct_rcd.t_ray);
        inct->geometry_coord = FCoord(x_, cross(z_, x_), z_);
        inct->uv             = t_a_ + inct_rcd.uv.x * t_b_a_
                                    + inct_rcd.uv.y * t_c_a_;
        inct->user_coord     = inct->geometry_coord;
        inct->wr             = -local_r.d;
        inct->t              = inct_rcd.t_ray;

        to_world(inct);

        return true;
    }

    AABB world_bound() const noexcept override
    {
        AABB ret;
        ret |= local_to_world_.apply_to_point(a_);
        ret |= local_to_world_.apply_to_point(a_ + b_a_);
        ret |= local_to_world_.apply_to_point(a_ + c_a_);
        for(int i = 0; i != 3; ++i)
        {
            if(ret.low[i] >= ret.high[i])
                ret.low[i] = ret.high[i] - real(0.1) * std::abs(ret.high[i]);
        }
        return ret;
    }

    real surface_area() const noexcept override
    {
        return surface_area_;
    }

    SurfacePoint sample(real *pdf, const Sample3 &sam) const noexcept override
    {
        const Vec2 bi_coord = math::distribution
                                ::uniform_on_triangle(sam.u, sam.v);

        SurfacePoint spt;
        spt.pos            = a_ + bi_coord.x * b_a_ + bi_coord.y * c_a_;
        spt.geometry_coord = FCoord(x_, cross(z_, x_), z_);
        spt.uv             = t_a_ + bi_coord.x * t_b_a_ + bi_coord.y * t_c_a_;
        spt.user_coord     = spt.geometry_coord;

        to_world(&spt);

        *pdf = 1 / surface_area_;
        return spt;
    }

    SurfacePoint sample(
        const FVec3 &, real *pdf, const Sample3 &sam) const noexcept override
    {
        return sample(pdf, sam);
    }

    real pdf(const FVec3 &) const noexcept override
    {
        return 1 / surface_area_;
    }

    real pdf(const FVec3 &, const FVec3 &sample) const noexcept override
    {
        return pdf(sample);
    }

private:

    void init_from_params(const Params &params)
    {
        init_transform(params.local_to_world);

        a_   = params.a;
        b_a_ = params.b - params.a;
        c_a_ = params.c - params.a;

        t_a_   = params.t_a;
        t_b_a_ = params.t_b - params.t_a;
        t_c_a_ = params.t_c - params.t_a;

        z_ = cross(b_a_, c_a_).normalize();
        x_ = dpdu_as_ex(b_a_, c_a_, t_b_a_, t_c_a_, z_);

        const FVec3 world_b_a = local_to_world_.apply_to_vector(b_a_);
        const FVec3 world_c_a = local_to_world_.apply_to_vector(c_a_);
        surface_area_ = triangle_area(world_b_a, world_c_a);
    }

    Params params_;

    FVec3 a_, b_a_, c_a_;
    Vec2 t_a_, t_b_a_, t_c_a_;
    FVec3 x_, z_;
    real surface_area_ = 1;

};

RC<Geometry> create_triangle(
    const FVec3 &a, const FVec3 &b, const FVec3 &c,
    const Vec2 &t_a, const Vec2 &t_b, const Vec2 &t_c,
    const FTransform3 &local_to_world)
{
    Triangle::Params params = { a, b, c, t_a, t_b, t_c, local_to_world };
    return newRC<Triangle>(params);
}

AGZ_TRACER_END
