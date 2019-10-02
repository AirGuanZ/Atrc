#include <agz/tracer/core/entity.h>
#include <agz/tracer/core/geometry.h>
#include <agz/tracer/core/light.h>
#include <agz/tracer/core/medium_interface.h>
#include <agz/tracer/factory/raw/material.h>

AGZ_TRACER_BEGIN

namespace
{
    class DiffuseLight : public AreaLight
    {
        std::shared_ptr<const Geometry> geometry_;
        Spectrum radiance_;
        MediumInterface medium_interface_;

    public:

        using AreaLight::AreaLight;

        void initialize(std::shared_ptr<const Geometry> geometry, const Spectrum &radiance, const MediumInterface &med)
        {
            geometry_ = std::move(geometry);
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
            return geometry_.get();
        }

        const MediumInterface &medium_interface() const noexcept
        {
            return medium_interface_;
        }
    };
}

class DiffuseLightEntity : public Entity
{
    std::unique_ptr<DiffuseLight> light_;
    std::shared_ptr<Material> material_;

public:

    void initialize(
        std::shared_ptr<const Geometry> geometry, const Spectrum &radiance, const MediumInterface &med)
    {
        //light_ = arena.create<DiffuseLight>();
        light_ = std::make_unique<DiffuseLight>();
        light_->initialize(std::move(geometry), radiance, med);
        material_ = create_ideal_black();
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
        inct->material = material_.get();
        inct->medium_in  = light_->medium_interface().in.get();
        inct->medium_out = light_->medium_interface().out.get();
        return true;
    }

    AABB world_bound() const noexcept override
    {
        return light_->geometry()->world_bound();
    }

    const AreaLight *as_light() const noexcept override
    {
        return light_.get();
    }

    AreaLight *as_light() noexcept override
    {
        return light_.get();
    }
};

std::shared_ptr<Entity> create_diffuse_light(
    std::shared_ptr<const Geometry> geometry,
    const Spectrum &radiance,
    const MediumInterface &med)
{
    auto ret = std::make_shared<DiffuseLightEntity>();
    ret->initialize(geometry, radiance, med);
    return ret;
}

AGZ_TRACER_END
