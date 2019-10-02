#pragma once

#include <agz/tracer/core/path_tracing_integrator.h>

AGZ_TRACER_BEGIN

std::shared_ptr<PathTracingIntegrator> create_mis_integrator(
    int min_depth, int max_depth, real cont_prob);

std::shared_ptr<PathTracingIntegrator> create_native_integrator(
    int min_depth, int max_depth, real cont_prob);

AGZ_TRACER_END
