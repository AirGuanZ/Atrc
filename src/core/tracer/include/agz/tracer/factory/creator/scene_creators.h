#pragma once

#include <agz/tracer/factory/factory.h>
#include <agz/tracer/factory/raw/sampler.h>

AGZ_TRACER_FACTORY_BEGIN

void initialize_scene_factory(Factory<Scene> &factory);

std::shared_ptr<Scene> build_scene(const ConfigGroup &params, CreatingContext &context);

AGZ_TRACER_FACTORY_END
