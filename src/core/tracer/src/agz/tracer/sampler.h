#pragma once

#include <agz/tracer/core/sampler.h>

AGZ_TRACER_BEGIN

Sampler *create_native_sampler(
    int spp, int seed, bool use_time_seed,
    obj::Object::CustomedFlag customed_flag,
    Arena &arena);

AGZ_TRACER_END
