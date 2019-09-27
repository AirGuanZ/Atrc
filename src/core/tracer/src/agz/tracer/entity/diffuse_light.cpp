#include <agz/tracer/core/entity.h>
#include <agz/tracer/core/geometry.h>
#include <agz/tracer/core/light.h>

#include "../material/ideal_black.h"
#include "./medium_interface.h"

AGZ_TRACER_BEGIN

namespace
{
    class DiffuseLight : public AreaLight
    {
        const Geometry *geometry_ = nullptr;
        Spectrum radiance_;
        MediumInterface medium_interface_;

    public:

        using AreaLight::AreaLight;

        void initialize(const Geometry *geometry, const Spectrum &radiance, const MediumInterface &med)
        {
            geometry_ = geometry;
            radiance_ = radiance;
            medium_interface_ = med;
        }

        AreaLightSampleResult sample(const Vec3 &ref, const Sample5 &sam) const noexcept override
        {
            real pdf_area;
            auto spt = geometry_->sample(ref, &pdf_area, { sam.u, sam.v, sam.r });

            if(dot(spt.geometry_coord.z, ref - spt.pos) <= 0)
                return AREA_LIGHT_SAMPLE_RESULT_NULL;

            Vec3 spt_to_ref = ref - spt.pos;
            real dist2 = spt_to_ref.length_square();

            AreaLightSampleResult ret;
            ret.spt      = spt;
            ret.radiance = radiance_;
            ret.pdf      = pdf_area * dist2 / std::abs(cos(spt.geometry_coord.z, spt_to_ref));
            ret.is_delta = false;

            return ret;
        }

        real pdf(const Vec3 &ref, const SurfacePoint &spt) const noexcept override
        {
            if(dot(spt.geometry_coord.z, ref - spt.pos) <= 0)
                return 0;

            real area_pdf = geometry_->pdf(ref, spt.pos);
            Vec3 spt_to_ref = ref - spt.pos;
            real dist2 = spt_to_ref.length_square();
            real area_to_solid_angle_factor = dist2 / std::abs(cos(spt.geometry_coord.z, spt_to_ref));
            return area_pdf * area_to_solid_angle_factor;
        }

        Spectrum power() const noexcept override
        {
            return PI_r * radiance_ * geometry_->surface_area();
        }

        Spectrum radiance(const SurfacePoint &spt, const Vec3 &light_to_out) const noexcept override
        {
            return dot(spt.geometry_coord.z, light_to_out) > 0 ? radiance_ : Spectrum();
        }

        void preprocess(const Scene&) override
        {
            // do nothing
        }

        const Geometry *geometry() const noexcept
        {
            return geometry_;
        }

        const MediumInterface &medium_interface() const noexcept
        {
            return medium_interface_;
        }
    };
}

class DiffuseLightEntity : public Entity
{
    DiffuseLight *light_ = nullptr;

public:

    explicit DiffuseLightEntity(CustomedFlag customed_flag = DEFAULT_CUSTOMED_FLAG)
    {
        object_customed_flag_ = customed_flag;
    }

    static std::string description()
    {
        return R"___(
diffuse [Entity]
    geometry   [Geometry] geometry object
    radiance   [Spectrum] radiance value
    med_in/out [MediumInterface] (optional; defaultly set to void) standard medium interface
)___";
    }

    void initialize(const Config &params, obj::ObjectInitContext &init_ctx) override
    {
        AGZ_HIERARCHY_TRY

        init_customed_flag(params);

        auto geometry = GeometryFactory.create(params.child_group("geometry"), init_ctx);
        auto radiance = params.child_spectrum("radiance");
        
        MediumInterface med;
        med.initialize(params, init_ctx);

        initialize(geometry, radiance, med, *init_ctx.arena);

        AGZ_HIERARCHY_WRAP("in initializing geometric diffuse light object")
    }

    void initialize(
        const Geometry *geometry, const Spectrum &radiance, const MediumInterface &med, Arena &arena)
    {
        light_ = arena.create<DiffuseLight>();
        light_->initialize(geometry, radiance, med);
    }

    bool has_intersection(const Ray &r) const noexcept override
    {
        return light_->geometry()->has_intersection(r);
    }

    bool closest_intersection(const Ray &r, EntityIntersection *inct) const noexcept override
    {
        if(!light_->geometry()->closest_intersection(r, inct))
            return false;
        inct->entity   = this;
        inct->material = &IdealBlack::IDEAL_BLACK_INSTANCE();
        inct->medium_in  = light_->medium_interface().in;
        inct->medium_out = light_->medium_interface().out;
        return true;
    }

    AABB world_bound() const noexcept override
    {
        return light_->geometry()->world_bound();
    }

    const AreaLight *as_light() const noexcept override
    {
        return light_;
    }

    AreaLight *as_light() noexcept override
    {
        return light_;
    }
};

Entity *create_diffuse_light(
    const Geometry *geometry,
    const Spectrum &radiance,
    const MediumInterface &med,
    obj::Object::CustomedFlag customed_flag,
    Arena &arena)
{
    auto ret = arena.create<DiffuseLightEntity>(customed_flag);
    ret->initialize(geometry, radiance, med, arena);
    return ret;
}

AGZT_IMPLEMENTATION(Entity, DiffuseLightEntity, "diffuse")

AGZ_TRACER_END
