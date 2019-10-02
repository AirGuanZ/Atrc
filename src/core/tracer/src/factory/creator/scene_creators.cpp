#include <agz/tracer/core/entity.h>
#include <agz/tracer/core/light.h>
#include <agz/tracer/core/scene.h>
#include <agz/tracer/factory/creator/scene_creators.h>
#include <agz/tracer/factory/raw/aggregate.h>
#include <agz/tracer/factory/raw/envir_light.h>
#include <agz/tracer/factory/raw/scene.h>
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

        std::shared_ptr<Scene> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            return create_default_scene();
        }
    };

} // namespace scene

void initialize_scene_factory(Factory<Scene> &factory)
{
    factory.add_creator(std::make_unique<scene::DefaultSceneCreator>());
}

std::shared_ptr<Scene> build_scene(const ConfigGroup &params, CreatingContext &context)
{
    AGZ_HIERARCHY_TRY

    auto scene = context.create<Scene>(params);
    std::vector<std::shared_ptr<const Entity>> entities;

    if(auto ent_arr = params.find_child_array("entities"))
    {
        if(ent_arr->size() == 1)
            AGZ_LOG1("creating 1 entity");
        else
            AGZ_LOG1("creating ", ent_arr->size(), " entities");

        for(size_t i = 0; i < ent_arr->size(); ++i)
        {
            auto &group = ent_arr->at_group(i);
            if(stdstr::ends_with(group.child_str("type"), "//"))
                continue;

            auto entity = context.create<Entity>(group);
            entities.push_back(entity);
            if(auto light = entity->as_light())
                scene->add_light(light);
        }
    }

    if(auto ent_grp = params.find_child_group("named_entities"))
    {
        if(ent_grp->size() == 1)
            AGZ_LOG1("creating 1 named entity");
        else if(ent_grp->size() > 1)
            AGZ_LOG1("creating ", ent_grp->size(), " named entities");

        for(auto &grp : *ent_grp)
        {
            if(stdstr::ends_with(grp.first, "//"))
                continue;

            auto &group = grp.second->as_group();
            if(stdstr::ends_with(group.child_str("type"), "//"))
                continue;

            auto entity = context.create<Entity>(group);
            entities.push_back(entity);
            if(auto light = entity->as_light())
                scene->add_light(light);
        }
    }

    if(auto group = params.find_child_group("env"))
    {
        AGZ_LOG1("creating environment light");
        auto env = context.create<EnvirLight>(*group);
        scene->set_env_light(env);
    }
    else
        scene->set_env_light(create_no_env());

    AGZ_LOG1("creating entity aggregate");
    std::shared_ptr<Aggregate> aggregate;
    if(auto node = params.find_child_group("aggregate"))
        aggregate = context.create<Aggregate>(*node);
    else
        aggregate = create_native_aggregate();
    aggregate->build(entities);
    scene->set_aggregate(aggregate);

    return scene;

    AGZ_HIERARCHY_WRAP("in building scene")
}

AGZ_TRACER_FACTORY_END
