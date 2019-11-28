#include <agz/tracer/core/medium_interface.h>
#include <agz/tracer/factory/creator/entity_creators.h>
#include <agz/tracer/factory/raw/entity.h>
#include <agz/tracer/factory/raw/medium.h>

AGZ_TRACER_FACTORY_BEGIN

namespace entity
{

    MediumInterface create_medium_interface(const ConfigGroup &params, CreatingContext &context)
    {
        MediumInterface ret;

        if(auto node = params.find_child_group("med_in"))
            ret.in = context.create<Medium>(node->as_group(), context);
        else
            ret.in = create_void();

        if(auto node = params.find_child_group("med_out"))
            ret.out = context.create<Medium>(node->as_group(), context);
        else
            ret.out = create_void();

        return ret;
    }
    
    class DiffuseLightEntityCreator : public Creator<Entity>
    {
    public:

        std::string name() const override
        {
            return "diffuse";
        }

        std::shared_ptr<Entity> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            auto geometry = context.create<Geometry>(params.child_group("geometry"), context);
            auto radiance = params.child_spectrum("radiance");
            auto med = create_medium_interface(params, context);
            auto no_denoise = params.child_int_or("no_denoise", 0);
            return create_diffuse_light(std::move(geometry), radiance, med, no_denoise);
        }
    };

    class GeometricEntityCreator : public Creator<Entity>
    {
    public:

        std::string name() const override
        {
            return "geometric";
        }

        std::shared_ptr<Entity> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            auto geometry = context.create<Geometry>(params.child_group("geometry"), context);
            auto material = context.create<Material>(params.child_group("material"), context);
            auto med = create_medium_interface(params, context);
            bool is_shadow_catcher = params.child_int_or("shadow_catcher", 0) != 0;
            auto no_denoise = params.child_int_or("no_denoise", 0);
            return create_geometric(std::move(geometry), std::move(material), med,
                                    is_shadow_catcher, no_denoise);
        }
    };

} // namespace entity

void initialize_entity_factory(Factory<Entity> &factory)
{
    factory.add_creator(std::make_unique<entity::DiffuseLightEntityCreator>());
    factory.add_creator(std::make_unique<entity::GeometricEntityCreator>());
}

AGZ_TRACER_FACTORY_END
