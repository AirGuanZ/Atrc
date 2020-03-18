#include <agz/tracer/core/entity.h>
#include <agz/tracer/core/light.h>
#include <agz/tracer/core/scene.h>
#include <agz/factory/creator/scene_creators.h>
#include <agz/tracer/create/aggregate.h>
#include <agz/tracer/create/scene.h>
#include <agz/tracer/utility/logger.h>
#include <agz/utility/string.h>

AGZ_TRACER_FACTORY_BEGIN

namespace scene
{
    
    class DefaultSceneCreator : public Creator<Scene>
    {
    public:

        std::string name() const override
        {
            return "default";
        }

        RC<Scene> create(
            const ConfigGroup &params, CreatingContext &context) const override
        {
            DefaultSceneParams scene_params;

            if(auto ent_arr = params.find_child_array("entities"))
            {
                if(ent_arr->size() == 1)
                    AGZ_INFO("creating 1 entity");
                else
                    AGZ_INFO("creating {} entities", ent_arr->size());

                for(size_t i = 0; i < ent_arr->size(); ++i)
                {
                    auto &group = ent_arr->at_group(i);
                    if(stdstr::ends_with(group.child_str("type"), "//"))
                    {
                        AGZ_INFO("skip entity with type ending with //");
                        continue;
                    }

                    auto ent = context.create<Entity>(group);
                    scene_params.entities.push_back(ent);
                }
            }

            if(auto group = params.find_child_group("env"))
                scene_params.envir_light = context.create<EnvirLight>(*group);

            if(auto group = params.find_child_group("aggregate"))
                scene_params.aggregate = context.create<Aggregate>(*group);
            else
                scene_params.aggregate = create_native_aggregate();

            std::vector<RC<const Entity>> const_entities;
            const_entities.reserve(scene_params.entities.size());
            for(auto ent : scene_params.entities)
                const_entities.push_back(ent);
            scene_params.aggregate->build(const_entities);

            return create_default_scene(scene_params);
        }
    };

} // namespace scene

void initialize_scene_factory(Factory<Scene> &factory)
{
    factory.add_creator(newBox<scene::DefaultSceneCreator>());
}

AGZ_TRACER_FACTORY_END
