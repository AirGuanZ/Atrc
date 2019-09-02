#include <agz/tracer/utility/triangle_aux.h>

#include "transformed_geometry.h"

AGZ_TRACER_BEGIN

class Quad : public TransformedGeometry
{
    Vec3 a_;
    Vec3 b_a_, c_a_, d_a_;

    Vec2 t_a_;
    Vec2 t_b_a_, t_c_a_, t_d_a_;

    Vec3 x_abc_, x_acd_, z_;

    real surface_area_ = 1;
    real sample_abc_prob_ = 0;

public:

    using TransformedGeometry::TransformedGeometry;

    static std::string description()
    {
        return R"___(
quad [Geometry]
    transform [Transform[]] transform sequence
    A  [Vec3] quad vertex A
    B  [Vec3] quad vertex B
    C  [Vec3] quad vertex C
    D  [Vec3] quad vertex D
    tA [Vec2] uv at A
    tB [Vec2] uv at B
    tC [Vec2] uv at C
    tD [Vec2] uv at D

    constructed with triangle ABC and ACD
)___";
    }

    void initialize(const Config &params, obj::ObjectInitContext &init_ctx) override
    {
        AGZ_HIERARCHY_TRY

        init_transform(params);

        Vec3 a = params.child_vec3("A");
        Vec3 b = params.child_vec3("B");
        Vec3 c = params.child_vec3("C");
        Vec3 d = params.child_vec3("D");

        Vec2 t_a = params.child_vec2("tA");
        Vec2 t_b = params.child_vec2("tB");
        Vec2 t_c = params.child_vec2("tC");
        Vec2 t_d = params.child_vec2("tD");

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

        Vec3 world_b_a = local_to_world_.apply_to_vector(b_a_);
        Vec3 world_c_a = local_to_world_.apply_to_vector(c_a_);
        Vec3 world_d_a = local_to_world_.apply_to_vector(d_a_);

        real area_abc = triangle_area(world_b_a, world_c_a);
        real area_acd = triangle_area(world_c_a, world_d_a);
        surface_area_ = area_abc + area_acd;
        sample_abc_prob_ = area_abc / surface_area_;

        AGZ_HIERARCHY_WRAP("in initializing quad geometry object")
    }

    bool has_intersection(const Ray &r) const noexcept override
    {
        Ray local_r = to_local(r);
        return has_intersection_with_triangle(local_r, a_, b_a_, c_a_) ||
               has_intersection_with_triangle(local_r, a_, c_a_, d_a_);
    }

    bool closest_intersection(const Ray &r, GeometryIntersection *inct) const noexcept override
    {
        Ray local_r = to_local(r);
        TriangleIntersectionRecord inct_rcd;

        if(closest_intersection_with_triangle(local_r, a_, b_a_, c_a_, &inct_rcd))
        {
            inct->pos            = local_r.at(inct_rcd.t_ray);
            inct->geometry_coord = Coord(x_abc_, cross(z_, x_abc_), z_);
            inct->uv             = t_a_ + inct_rcd.uv.x * t_b_a_ + inct_rcd.uv.y * t_c_a_;
            inct->user_coord     = inct->geometry_coord;
            inct->wr             = -local_r.d;
            inct->t              = inct_rcd.t_ray;

            to_world(inct);

            return true;
        }
        
        if(closest_intersection_with_triangle(local_r, a_, c_a_, d_a_, &inct_rcd))
        {
            inct->pos            = local_r.at(inct_rcd.t_ray);
            inct->geometry_coord = Coord(x_acd_, cross(z_, x_acd_), z_);
            inct->uv             = t_a_ + inct_rcd.uv.x * t_c_a_ + inct_rcd.uv.y * t_d_a_;
            inct->user_coord     = inct->geometry_coord;
            inct->wr             = -r.d;
            inct->t              = inct_rcd.t_ray;

            to_world(inct);

            return true;
        }

        return false;
    }

    AABB world_bound() const noexcept override
    {
        AABB ret;
        ret |= local_to_world_.apply_to_point(a_);
        ret |= local_to_world_.apply_to_point(a_ + b_a_);
        ret |= local_to_world_.apply_to_point(a_ + c_a_);
        ret |= local_to_world_.apply_to_point(a_ + d_a_);
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
        Vec2 bi_coord = math::distribution::uniform_on_triangle(sam.u, sam.v);
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

        to_world(&spt);
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

AGZT_IMPLEMENTATION(Geometry, Quad, "quad")

AGZ_TRACER_END
