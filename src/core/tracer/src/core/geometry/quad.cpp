#include <agz/tracer/core/geometry.h>
#include <agz/tracer/utility/triangle_aux.h>
#include <agz/utility/misc.h>

AGZ_TRACER_BEGIN

class Quad : public Geometry
{
    Vec3 a_;
    Vec3 b_a_, c_a_, d_a_;

    Vec2 t_a_;
    Vec2 t_b_a_, t_c_a_, t_d_a_;

    Vec3 x_abc_, x_acd_, z_;

    real surface_area_ = 1;
    real sample_abc_prob_ = 0;

public:

    Quad(
        const Vec3 &A, const Vec3 &B, const Vec3 &C, const Vec3 &D,
        const Vec2 &t_a, const Vec2 &t_b, const Vec2 &t_c, const Vec2 &t_d,
        const Transform3 &local_to_world)
    {
        AGZ_HIERARCHY_TRY

        const Vec3 a = local_to_world.apply_to_point(A);
        const Vec3 b = local_to_world.apply_to_point(B);
        const Vec3 c = local_to_world.apply_to_point(C);
        const Vec3 d = local_to_world.apply_to_point(D);

        a_ = a;
        b_a_ = b - a;
        c_a_ = c - a;
        d_a_ = d - a;

        t_a_ = t_a;
        t_b_a_ = t_b - t_a;
        t_c_a_ = t_c - t_a;
        t_d_a_ = t_d - t_a;

        z_ = cross(b_a_, c_a_).normalize();
        x_abc_ = dpdu_as_ex(b_a_, c_a_, t_b_a_, t_c_a_, z_);

        if(std::abs(dot(cross(c_a_, d_a_).normalize(), z_) - 1) > 0.05)
            throw ObjectConstructionException("four quad vertices must be on one plane");
        x_acd_ = dpdu_as_ex(c_a_, d_a_, t_c_a_, t_d_a_, z_);

        const real area_abc = triangle_area(b_a_, c_a_);
        const real area_acd = triangle_area(c_a_, d_a_);
        surface_area_ = area_abc + area_acd;
        sample_abc_prob_ = area_abc / surface_area_;

        AGZ_HIERARCHY_WRAP("in initializing quad geometry object")
    }

    bool has_intersection(const Ray &r) const noexcept override
    {
        return has_intersection_with_triangle(r, a_, b_a_, c_a_) ||
               has_intersection_with_triangle(r, a_, c_a_, d_a_);
    }

    bool closest_intersection(const Ray &r, GeometryIntersection *inct) const noexcept override
    {
        TriangleIntersectionRecord inct_rcd;

        if(closest_intersection_with_triangle(r, a_, b_a_, c_a_, &inct_rcd))
        {
            inct->pos            = r.at(inct_rcd.t_ray);
            inct->geometry_coord = Coord(x_abc_, cross(z_, x_abc_), z_);
            inct->uv             = t_a_ + inct_rcd.uv.x * t_b_a_ + inct_rcd.uv.y * t_c_a_;
            inct->user_coord     = inct->geometry_coord;
            inct->wr             = -r.d;
            inct->t              = inct_rcd.t_ray;
            return true;
        }
        
        if(closest_intersection_with_triangle(r, a_, c_a_, d_a_, &inct_rcd))
        {
            inct->pos            = r.at(inct_rcd.t_ray);
            inct->geometry_coord = Coord(x_acd_, cross(z_, x_acd_), z_);
            inct->uv             = t_a_ + inct_rcd.uv.x * t_c_a_ + inct_rcd.uv.y * t_d_a_;
            inct->user_coord     = inct->geometry_coord;
            inct->wr             = -r.d;
            inct->t              = inct_rcd.t_ray;
            return true;
        }

        return false;
    }

    AABB world_bound() const noexcept override
    {
        AABB ret;
        ret |= a_;
        ret |= a_ + b_a_;
        ret |= a_ + c_a_;
        ret |= a_ + d_a_;
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
        const Vec2 bi_coord = math::distribution::uniform_on_triangle(sam.u, sam.v);
        SurfacePoint spt;

        if(sam.w < sample_abc_prob_)
        {
            spt.pos = a_ + bi_coord.x * b_a_ + bi_coord.y * c_a_;
            spt.geometry_coord = Coord(x_abc_, cross(z_, x_abc_), z_);

            spt.uv = t_a_ + bi_coord.x * t_b_a_ + bi_coord.y * t_c_a_;
            spt.user_coord = spt.geometry_coord;
        }
        else
        {
            spt.pos = a_ + bi_coord.x * c_a_ + bi_coord.y * d_a_;
            spt.geometry_coord = Coord(x_acd_, cross(z_, x_acd_), z_);

            spt.uv = t_a_ + bi_coord.x * t_c_a_ + bi_coord.y * t_d_a_;
            spt.user_coord = spt.geometry_coord;
        }

        *pdf = 1 / surface_area_;
        return spt;
    }

    SurfacePoint sample(const Vec3&, real *pdf, const Sample3 &sam) const noexcept override
    {
        return sample(pdf, sam);
    }

    real pdf(const Vec3&) const noexcept override
    {
        return 1 / surface_area_;
    }

    real pdf(const Vec3&, const Vec3 &sample) const noexcept override
    {
        return pdf(sample);
    }
};

std::shared_ptr<Geometry> create_quad(
    const Vec3 &a, const Vec3 &b, const Vec3 &c, const Vec3 &d,
    const Vec2 &t_a, const Vec2 &t_b, const Vec2 &t_c, const Vec2 &t_d,
    const Transform3 &local_to_world)
{
    return std::make_shared<Quad>(a, b, c, d, t_a, t_b, t_c, t_d, local_to_world);
}

AGZ_TRACER_END
