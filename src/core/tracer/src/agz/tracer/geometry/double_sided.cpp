#include <agz/tracer/core/geometry.h>

AGZ_TRACER_BEGIN

class DoubleSidedGeometry : public Geometry
{
    const Geometry *internal_ = nullptr;

public:

    explicit DoubleSidedGeometry(CustomedFlag customed_flag = DEFAULT_CUSTOMED_FLAG)
    {
        object_customed_flag_ = customed_flag;
    }

    static std::string description()
    {
        return R"___(
double_sided [Geometry]
    internal [Geometry] internal (wrapped) geometry object
)___";
    }

    void initialize(const Config &params, obj::ObjectInitContext &context) override
    {
        AGZ_HIERARCHY_TRY

        init_customed_flag(params);

        auto internal = GeometryFactory.create(params.child_group("internal"), context);

        initialize(internal);

        AGZ_HIERARCHY_WRAP("in initializing double-sided geometry")
    }

    void initialize(const Geometry *internal)
    {
        internal_ = internal;
    }

    bool has_intersection(const Ray &r) const noexcept override
    {
        return internal_->has_intersection(r);
    }

    bool closest_intersection(const Ray &r, GeometryIntersection *inct) const noexcept override
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

    AABB world_bound() const noexcept override
    {
        return internal_->world_bound();
    }

    real surface_area() const noexcept override
    {
        return 2 * internal_->surface_area();
    }

    SurfacePoint sample(real *pdf, const Sample3 &sam) const noexcept override
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

    SurfacePoint sample(const Vec3 &ref, real *pdf, const Sample3 &sam) const noexcept override
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

    real pdf(const Vec3 &sample) const noexcept override
    {
        auto internal_pdf = internal_->pdf(sample);
        return internal_pdf * real(0.5);
    }

    real pdf(const Vec3 &ref, const Vec3 &sample) const noexcept override
    {
        auto internal_pdf = internal_->pdf(ref, sample);
        return internal_pdf * real(0.5);
    }
};

Geometry *create_double_sided(
    const Geometry *internal,
    obj::Object::CustomedFlag customed_flag,
    Arena &arena)
{
    auto ret = arena.create<DoubleSidedGeometry>(customed_flag);
    ret->initialize(internal);
    return ret;
}

AGZT_IMPLEMENTATION(Geometry, DoubleSidedGeometry, "double_sided")

AGZ_TRACER_END
