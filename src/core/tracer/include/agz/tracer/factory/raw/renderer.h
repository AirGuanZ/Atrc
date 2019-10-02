#pragma once

#include <agz/tracer/core/path_tracing_integrator.h>
#include <agz/tracer/core/renderer.h>
#include <agz/tracer/core/sampler.h>

AGZ_TRACER_BEGIN

struct IsolatedPathTracerParams
{
    int min_depth = 5;
    int max_depth = 10;
    real cont_prob = real(0.9);

    int shading_aa = 1;
    int background_aa = 1;

    Spectrum background_color = Spectrum(1);
    bool hide_background_entity = false;

    int worker_count = 0;
    int task_grid_size = 32;

    std::shared_ptr<const Sampler> sampler_prototype;
};

std::shared_ptr<Renderer> create_isolated_path_tracer(
    const IsolatedPathTracerParams &params);

std::shared_ptr<Renderer> create_path_tracer(
    std::shared_ptr<const PathTracingIntegrator> integrator,
    int worker_count, int task_grid_size,
    std::shared_ptr<const Sampler> sampler_prototype);

AGZ_TRACER_END

