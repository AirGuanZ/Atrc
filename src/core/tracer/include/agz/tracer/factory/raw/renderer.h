#pragma once

#include <agz/tracer/core/path_tracing_integrator.h>
#include <agz/tracer/core/renderer.h>
#include <agz/tracer/core/sampler.h>

AGZ_TRACER_BEGIN

struct PathTracingRendererParams
{
    int worker_count = 0;
    std::shared_ptr<const Sampler> sampler_prototype;
    std::shared_ptr<const PathTracingIntegrator> integrator;
};

std::shared_ptr<Renderer> create_path_tracing_renderer(
    const PathTracingRendererParams &params);

AGZ_TRACER_END
