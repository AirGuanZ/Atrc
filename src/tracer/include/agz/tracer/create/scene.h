#pragma once

#include <agz/tracer/core/scene.h>

AGZ_TRACER_BEGIN

struct DefaultSceneParams
{
    std::vector<RC<Entity>> entities;
    RC<EnvirLight>          envir_light;
    RC<Aggregate>           aggregate;
};

RC<Scene> create_default_scene(const DefaultSceneParams &params);

AGZ_TRACER_END
