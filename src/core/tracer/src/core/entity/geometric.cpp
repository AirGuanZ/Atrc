#include <agz/tracer/core/entity.h>
#include <agz/tracer/core/geometry.h>
#include <agz/tracer/core/material.h>
#include <agz/tracer/core/medium_interface.h>

AGZ_TRACER_BEGIN

class GeometricEntity : public Entity
{
    std::shared_ptr<const Geometry> geometry_;
    std::shared_ptr<const Material> material_;
    MediumInterface medium_interface_;

    bool is_shadow_catcher_ = false;

public:

    void initialize(
        std::shared_ptr<const Geometry> geometry, std::shared_ptr<const Material> material, const MediumInterface &med,
        bool shadow_catcher, bool no_denoise)
    {
        geometry_ = std::move(geometry);
        material_ = std::move(material);
        medium_interface_ = med;
        is_shadow_catcher_ = shadow_catcher;
        set_no_denoise_flag(no_denoise);
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
        return nullptr;
    }

    AreaLight *as_light() noexcept override
    {
        return nullptr;
    }

    bool is_shadow_catcher() const noexcept override
    {
        return is_shadow_catcher_;
    }
};

std::shared_ptr<Entity> create_geometric(
    std::shared_ptr<const Geometry> geometry,
    std::shared_ptr<const Material> material,
    const MediumInterface &med,
    bool shadow_catcher, bool no_denoise)
{
    auto ret = std::make_shared<GeometricEntity>();
    ret->initialize(std::move(geometry), std::move(material), med,
                    shadow_catcher, no_denoise);
    return ret;
}

AGZ_TRACER_END
