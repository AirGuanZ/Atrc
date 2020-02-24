#include <agz/tracer/core/entity.h>
#include <agz/tracer/core/geometry.h>
#include <agz/tracer/core/material.h>
#include <agz/tracer/core/medium.h>

#include "area_light/geometry_to_diffuse_light.h"

AGZ_TRACER_BEGIN

class GeometricEntity : public Entity
{
    std::shared_ptr<const Geometry> geometry_;
    std::shared_ptr<const Material> material_;
    MediumInterface medium_interface_;

    std::unique_ptr<GeometryToDiffuseLight> diffuse_light_;

public:

    GeometricEntity(
        std::shared_ptr<const Geometry> geometry,
        std::shared_ptr<const Material> material,
        const MediumInterface &med,
        const Spectrum &emit_radiance,
        bool no_denoise)
    {
        geometry_ = std::move(geometry);
        material_ = std::move(material);
        medium_interface_ = med;
        
        set_no_denoise_flag(no_denoise);

        if(!emit_radiance.is_black())
            diffuse_light_ = std::make_unique<GeometryToDiffuseLight>(geometry_.get(), emit_radiance);
    }

    bool has_intersection(const Ray &r) const noexcept override
    {
        return geometry_->has_intersection(r);
    }

    bool closest_intersection(const Ray &r, EntityIntersection *inct) const noexcept override
    {
        if(!geometry_->closest_intersection(r, inct))
            return false;
        inct->entity     = this;
        inct->material   = material_.get();

        inct->medium_in  = medium_interface_.in.get();
        inct->medium_out = medium_interface_.out.get();

        return true;
    }

    AABB world_bound() const noexcept override
    {
        return geometry_->world_bound();
    }

    const AreaLight *as_light() const noexcept override
    {
        return diffuse_light_.get();
    }

    AreaLight *as_light() noexcept override
    {
        return diffuse_light_.get();
    }
};

std::shared_ptr<Entity> create_geometric(
    std::shared_ptr<const Geometry> geometry,
    std::shared_ptr<const Material> material,
    const MediumInterface &med,
    const Spectrum &emit_radiance,
    bool no_denoise)
{
    return std::make_shared<GeometricEntity>(
        std::move(geometry), std::move(material), med, emit_radiance, no_denoise);
}

AGZ_TRACER_END
