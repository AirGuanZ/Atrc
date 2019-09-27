#pragma once

#include <agz/tracer/core/path_tracing_integrator.h>
#include <agz/tracer/core/renderer.h>
#include <agz/tracer/core/sampler.h>

AGZ_TRACER_BEGIN

struct IsolatedPathTracerParams
{
    int min_depth;
    int max_depth;
    real cont_prob;

    int shading_aa;
    int background_aa;

    obj::Object::CustomedFlag background_entity_flag;
    Spectrum background_color;
    bool hide_background_entity;

    int worker_count;
    int task_grid_size;

    const Sampler *sampler_prototype;
};

Renderer *create_isolated_path_tracer(
    const IsolatedPathTracerParams &params,
    obj::Object::CustomedFlag customed_flag,
    Arena &arena);

Renderer *create_path_tracer(
    const PathTracingIntegrator *integrator,
    int worker_count, int task_grid_size,
    const Sampler *sampler_prototype,
    obj::Object::CustomedFlag customed_flag,
    Arena &arena);

AGZ_TRACER_END

