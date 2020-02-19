#pragma once

#include <agz/tracer/factory/factory.h>
#include <agz/tracer/factory/raw/sampler.h>

AGZ_TRACER_FACTORY_BEGIN

void initialize_sampler_factory(Factory<Sampler> &factory);

AGZ_TRACER_FACTORY_END
