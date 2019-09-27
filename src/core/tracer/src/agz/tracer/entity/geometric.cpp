#include <agz/tracer/core/entity.h>
#include <agz/tracer/core/geometry.h>
#include <agz/tracer/core/material.h>
#include "./medium_interface.h"

AGZ_TRACER_BEGIN

class GeometricEntity : public Entity
{
    const Geometry *geometry_ = nullptr;
    const Material *material_ = nullptr;
    MediumInterface medium_interface_;

public:

    explicit GeometricEntity(CustomedFlag customed_flag = DEFAULT_CUSTOMED_FLAG)
    {
        object_customed_flag_ = customed_flag;
    }

    static std::string description()
    {
        return R"___(
geometric [Entity]
    geometry [Geometry] geometry object
    material [Material] surface material
)___";
    }

    void initialize(const Config &params, obj::ObjectInitContext &context) override
    {
        AGZ_HIERARCHY_TRY

        init_customed_flag(params);

        auto geometry = GeometryFactory.create(params.child_group("geometry"), context);
        auto material = MaterialFactory.create(params.child_group("material"), context);

        MediumInterface med;
        med.initialize(params, context);

        initialize(geometry, material, med);

        AGZ_HIERARCHY_WRAP("in initializing geometric entity")
    }

    void initialize(const Geometry *geometry, const Material *material, const MediumInterface &med)
    {
        geometry_ = geometry;
        material_ = material;
        medium_interface_ = med;
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
        inct->material   = material_;

        inct->medium_in  = medium_interface_.in;
        inct->medium_out = medium_interface_.out;

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
};

Entity *create_geometric(
    const Geometry *geometry,
    const Material *material,
    const MediumInterface &med,
    obj::Object::CustomedFlag customed_flag,
    Arena &arena)
{
    auto ret = arena.create<GeometricEntity>(customed_flag);
    ret->initialize(geometry, material, med);
    return ret;
}

AGZT_IMPLEMENTATION(Entity, GeometricEntity, "geometric")

AGZ_TRACER_END
