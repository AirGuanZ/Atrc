#include <agz/tracer/core/aggregate.h>
#include <agz/tracer/core/camera.h>
#include <agz/tracer/core/entity.h>
#include <agz/tracer/core/light.h>
#include <agz/tracer/utility/logger.h>
#include <agz/tracer/utility/scene_builder.h>

#include "../envir_light/no_env.h"

AGZ_TRACER_BEGIN

Scene *SceneBuilder::build(const Config &params, obj::ObjectInitContext &init_ctx)
{
    AGZ_HIERARCHY_TRY

    auto scene = SceneFactory.create(params, init_ctx);
    std::vector<Entity*> entities;

    if(auto ent_arr = params.find_child_array("entities"))
    {
        if(ent_arr->size() == 1)
            AGZ_LOG1("creating 1 entity");
        else if(ent_arr->size() > 1)
            AGZ_LOG1("creating ", ent_arr->size(), " entities");

        for(size_t i = 0; i < ent_arr->size(); ++i)
        {
            auto group = ent_arr->at_group(i);
            if(stdstr::ends_with(group.child_str("type"), "//"))
                continue;

            auto entity = EntityFactory.create(group, init_ctx);
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

            auto entity = EntityFactory.create(group, init_ctx);
            entities.push_back(entity);
            if(auto light = entity->as_light())
                scene->add_light(light);
        }
    }

    if(auto group = params.find_child_group("env"))
    {
        AGZ_LOG1("creating environment light");
        auto env = EnvirLightFactory.create(*group, init_ctx);
        scene->set_env_light(env);
    }
    else
        scene->set_env_light(init_ctx.arena->create<NoEnv>());

    AGZ_LOG1("creating entity aggregate");
    Aggregate *aggregate;
    if(auto node = params.find_child_group("aggregate"))
    {
        aggregate = AggregateFactory.create(*node, init_ctx);
    }
    else
    {
        Config native_params;
        native_params.insert_child("type", std::make_shared<ConfigValue>("native"));
        aggregate = AggregateFactory.create(native_params, init_ctx);
    }
    aggregate->build(entities.data(), entities.size());
    scene->set_aggregate(aggregate);

    return scene;

    AGZ_HIERARCHY_WRAP("in building scene")
}

AGZ_TRACER_END
