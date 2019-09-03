#include <agz/tracer/core/entity.h>
#include <agz/tracer/core/geometry.h>
#include <agz/tracer/core/light.h>

#include "../material/ideal_black.h"
#include "./medium_interface.h"

AGZ_TRACER_BEGIN

namespace
{
    class DiffuseLight : public Light
    {
        const Geometry *geometry_ = nullptr;
        Spectrum radiance_;
        MediumInterface medium_interface_;

    public:

        using Light::Light;

        void initialize(const Config &params, obj::ObjectInitContext &init_ctx) override
        {
            geometry_ = GeometryFactory.create(params.child_group("geometry"), init_ctx);
            radiance_ = params.child_spectrum("radiance");
            medium_interface_.initialize(params, init_ctx);
        }

        LightSampleResult sample(const Vec3 &ref, const Sample5 &sam) const noexcept override
        {
            real pdf_area;
            auto spt = geometry_->sample(ref, &pdf_area, { sam.u, sam.v, sam.r });

            if(dot(spt.geometry_coord.z, ref - spt.pos) <= 0)
                return LIGHT_SAMPLE_RESULT_NULL;

            Vec3 spt_to_ref = ref - spt.pos;
            real dist2 = spt_to_ref.length_square();

            LightSampleResult ret;
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

        //LightEmitResult emit(const Sample5 &sam) const noexcept override
        //{
        //    real pdf_pos;
        //    auto spt = geometry_->sample(&pdf_pos, { sam.u, sam.v, sam.w });

        //    auto [local_dir, pdf_dir] = math::distribution::zweighted_on_hemisphere(sam.r, sam.s);
        //    Vec3 global_dir = Coord::from_z(spt.geometry_coord.z).local_to_global(local_dir);

        //    LightEmitResult ret;
        //    ret.spt      = spt;
        //    ret.dir      = global_dir;
        //    ret.radiance = radiance_;
        //    ret.pdf_pos  = pdf_pos;
        //    ret.pdf_dir  = pdf_dir;
        //    ret.medium   = medium_interface_.out;

        //    return ret;
        //}

        //void emit_pdf(const SurfacePoint &spt, const Vec3 &light_to_out, real *pdf_pos, real *pdf_dir) const noexcept override
        //{
        //    *pdf_pos = geometry_->pdf(spt.pos);

        //    auto local_dir = Coord::from_z(spt.geometry_coord.z).global_to_local(light_to_out).normalize();
        //    if(local_dir.z <= 0)
        //    {
        //        *pdf_dir = 0;
        //        return;
        //    }

        //    *pdf_dir = math::distribution::zweighted_on_hemisphere_pdf(local_dir.z);
        //}

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

    using Entity::Entity;

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

        light_ = init_ctx.arena->create<DiffuseLight>();
        light_->initialize(params, init_ctx);

        AGZ_HIERARCHY_WRAP("in initializing geometric diffuse light object")
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

    const Light *as_light() const noexcept override
    {
        return light_;
    }

    Light *as_light() noexcept override
    {
        return light_;
    }
};

AGZT_IMPLEMENTATION(Entity, DiffuseLightEntity, "diffuse")

AGZ_TRACER_END
