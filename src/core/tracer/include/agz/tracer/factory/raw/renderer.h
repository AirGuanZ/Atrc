#pragma once

#include <agz/tracer/core/path_tracing_integrator.h>
#include <agz/tracer/core/renderer.h>
#include <agz/tracer/core/sampler.h>

AGZ_TRACER_BEGIN

struct PathTracingRendererParams
{
    int worker_count   = 0;
    int task_grid_size = 32;
    std::shared_ptr<const Sampler>               sampler_prototype;
    std::shared_ptr<const PathTracingIntegrator> integrator;
};

struct ParticleTracingRendererParams
{
    int worker_count = 0;

    // backward tracing

    int particle_task_count = 1;
    std::shared_ptr<const Sampler> particle_sampler_prototype;

    int min_depth = 5;
    int max_depth = 10;
    real cont_prob = real(0.9);

    // forward tracing

    int forward_task_grid_size = 32;
    std::shared_ptr<const Sampler> forward_sampler_prototype;
};

std::shared_ptr<Renderer> create_path_tracing_renderer(
    const PathTracingRendererParams &params);

std::shared_ptr<Renderer> create_particle_tracing_renderer(
    const ParticleTracingRendererParams &params);

AGZ_TRACER_END
