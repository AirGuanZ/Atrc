#include <agz/tracer/core/geometry.h>

AGZ_TRACER_BEGIN

class DoubleSidedGeometry : public Geometry
{
    Geometry *internal_ = nullptr;

public:

    using Geometry::Geometry;

    static std::string description()
    {
        return R"___(
double_sided [Geometry]
    internal [Geometry] internal (wrapped) geometry object
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

AGZT_IMPLEMENTATION(Geometry, DoubleSidedGeometry, "double_sided")

void DoubleSidedGeometry::initialize(const Config &params, obj::ObjectInitContext &context)
{
    AGZ_HIERARCHY_TRY

    internal_ = GeometryFactory.create(params.child_group("internal"), context);

    AGZ_HIERARCHY_WRAP("in initializing double-sided geometry")
}

bool DoubleSidedGeometry::has_intersection(const Ray &r) const noexcept
{
    return internal_->has_intersection(r);
}

bool DoubleSidedGeometry::closest_intersection(const Ray &r, GeometryIntersection *inct) const noexcept
{
    if(!internal_->closest_intersection(r, inct))
        return false;

    bool backface = dot(inct->geometry_coord.z, inct->wr) < 0;
    if(backface)
    {
        inct->geometry_coord = -inct->geometry_coord;
        inct->user_coord     = -inct->user_coord;
    }

    return true;
}

AABB DoubleSidedGeometry::world_bound() const noexcept
{
    return internal_->world_bound();
}

real DoubleSidedGeometry::surface_area() const noexcept
{
    return 2 * internal_->surface_area();
}

SurfacePoint DoubleSidedGeometry::sample(real *pdf, const Sample3 &sam) const noexcept
{
    Sample3 new_sam = sam;
    bool backface = new_sam.u < real(0.5);
    if(backface)
        new_sam.u = 2 * sam.u;
    else
        new_sam.u = 2 * (sam.u - real(0.5));

    SurfacePoint spt = internal_->sample(pdf, new_sam);
    *pdf = *pdf * real(0.5);
    if(backface)
    {
        spt.geometry_coord = -spt.geometry_coord;
        spt.user_coord     = -spt.user_coord;
    }

    return spt;
}

SurfacePoint DoubleSidedGeometry::sample(const Vec3 &ref, real *pdf, const Sample3 &sam) const noexcept
{
    Sample3 new_sam = sam;
    bool backface = new_sam.u < real(0.5);
    if(backface)
        new_sam.u = 2 * sam.u;
    else
        new_sam.u = 2 * (sam.u - real(0.5));

    SurfacePoint spt = internal_->sample(ref, pdf, new_sam);
    *pdf = *pdf * real(0.5);
    if(backface)
    {
        spt.geometry_coord = -spt.geometry_coord;
        spt.user_coord     = -spt.user_coord;
    }

    return spt;
}

real DoubleSidedGeometry::pdf(const Vec3 &sample) const noexcept
{
    auto internal_pdf = internal_->pdf(sample);
    return internal_pdf * real(0.5);
}

real DoubleSidedGeometry::pdf(const Vec3 &ref, const Vec3 &sample) const noexcept
{
    auto internal_pdf = internal_->pdf(ref, sample);
    return internal_pdf * real(0.5);
}

AGZ_TRACER_END
