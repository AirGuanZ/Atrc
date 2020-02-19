#pragma once

#include <agz/tracer/core/scene.h>

AGZ_TRACER_BEGIN

struct DefaultSceneParams
{
    std::vector<std::shared_ptr<Entity>> entities;
    std::shared_ptr<EnvirLight>          envir_light;
    std::shared_ptr<Aggregate>           aggregate;
};

std::shared_ptr<Scene> create_default_scene(const DefaultSceneParams &params);

AGZ_TRACER_END
