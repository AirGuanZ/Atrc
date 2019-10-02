#pragma once

#include <agz/tracer/core/sampler.h>

AGZ_TRACER_BEGIN

std::shared_ptr<Sampler> create_native_sampler(
    int spp, int seed, bool use_time_seed);

AGZ_TRACER_END
