#include <agz/tracer/utility/sphere_aux.h>
#include "transformed_geometry.h"

AGZ_TRACER_BEGIN

class Sphere : public TransformedGeometry
{
    real radius_ = 1;
    real world_radius_ = 1;

public:

    using TransformedGeometry::TransformedGeometry;

    static std::string description()
    {
        return R"___(
sphere [Geometry]
    transform [Transform[]] transform sequence
    radius    [real]        sphere radius
)___";
    }

    void initialize(const Config &params, obj::ObjectInitContext &context) override;

    bool has_intersection(const Ray &r) const noexcept override;

    bool closest_intersection(const Ray &r, GeometryIntersection *inct) const noexcept override;

    AABB world_bound() const noexcept override;

    real surface_area() const noexcept override;

    SurfacePoint sample(real *pdf, const Sample3 &sam) const noexcept override;

    SurfacePoint sample(const Vec3 &ref, real *pdf, const Sample3 &sam) const noexcept override;

    real pdf(const Vec3 &sample) const noexcept override;

    real pdf(const Vec3 &ref, const Vec3 &sample) const noexcept override;
};

AGZT_IMPLEMENTATION(Geometry, Sphere, "sphere")

void Sphere::initialize(const Config &params, obj::ObjectInitContext&)
{
    AGZ_HIERARCHY_TRY

    init_customed_flag(params);

    init_transform(params);

    radius_ = params.child_real("radius");
    if(radius_ <= 0)
        throw ObjectConstructionException("invalid sphere radius: " + std::to_string(radius_));

    world_radius_ = local_to_world_ratio_ * radius_;

    AGZ_HIERARCHY_WRAP("in initializing sphere")
}

bool Sphere::has_intersection(const Ray &r) const noexcept
{
    Ray local_r = to_local(r);
    return sphere::has_intersection(local_r, radius_);
}

bool Sphere::closest_intersection(const Ray &r, GeometryIntersection *inct) const noexcept
{
    Ray local_r = to_local(r);

    real t;
    if(!sphere::closest_intersection(local_r, &t, radius_))
        return false;

    Vec3 pos = local_r.at(t);

    Vec2 geometry_uv(UNINIT);
    Coord geometry_coord(UNINIT);
    sphere::local_geometry_uv_and_coord(
        pos, &geometry_uv, &geometry_coord, radius_);

    inct->pos            = pos;
    inct->geometry_coord = geometry_coord;
    inct->uv             = geometry_uv;
    inct->user_coord     = geometry_coord;
    inct->wr             = -local_r.d;
    inct->t              = t;

    to_world(inct);

    return true;
}

AABB Sphere::world_bound() const noexcept
{
    Vec3 world_origin = local_to_world_.apply_to_point(Vec3(0));
    return {
        world_origin - Vec3(world_radius_ + EPS),
        world_origin + Vec3(world_radius_ + EPS)
    };
}

real Sphere::surface_area() const noexcept
{
    return 4 * PI_r * world_radius_ * world_radius_;
}

SurfacePoint Sphere::sample(real *pdf, const Sample3 &sam) const noexcept
{
    assert(pdf);

    auto [unit_pos, unit_pdf] = math::distribution::uniform_on_sphere(sam.u, sam.v);
    *pdf = unit_pdf / (world_radius_ * world_radius_);
    Vec3 pos = radius_ * unit_pos;

    Vec2 geometry_uv(UNINIT); Coord geometry_coord(UNINIT);
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

SurfacePoint Sphere::sample(const Vec3 &ref, real *pdf, const Sample3 &sam) const noexcept
{
    Vec3 local_ref = local_to_world_.apply_inverse_to_point(ref);
    real d = local_ref.length();
    if(d <= radius_)
        return sample(pdf, sam);

    real cos_theta = (std::min)(radius_ / d, real(1));
    auto [dir, l_pdf] = math::distribution::uniform_on_cone(cos_theta, sam.u, sam.v);
    Vec3 pos = radius_ * Coord::from_z(local_ref).local_to_global(dir).normalize();

    Vec2 geometry_uv; Coord geometry_coord;
    sphere::local_geometry_uv_and_coord(
        pos, &geometry_uv, &geometry_coord, radius_);

    SurfacePoint spt;
    spt.pos            = pos;
    spt.geometry_coord = geometry_coord;
    spt.uv             = geometry_uv;
    spt.user_coord     = geometry_coord;
    to_world(&spt);

    real world_radius_square = world_radius_ * world_radius_;
    *pdf = l_pdf / world_radius_square;

    return spt;
}

real Sphere::pdf(const Vec3&) const noexcept
{
    return 1 / surface_area();
}

real Sphere::pdf(const Vec3 &ref, const Vec3 &sample) const noexcept
{
    Vec3 local_ref = local_to_world_.apply_inverse_to_point(ref);
    real d = local_ref.length();
    if(d <= radius_)
        return pdf(sample);

    real cos_theta = (std::min)(radius_ / d, real(1));
    real world_radius_square = world_radius_ * world_radius_;
    return math::distribution::uniform_on_cone_pdf(cos_theta) / world_radius_square;
}

AGZ_TRACER_END
