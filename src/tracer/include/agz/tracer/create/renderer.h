#pragma once

#include <agz/tracer/core/renderer.h>
#include <agz/tracer/core/sampler.h>

AGZ_TRACER_BEGIN

// path tracing

struct PathTracingRendererParams
{
    int min_depth  = 5;
    int max_depth  = 10;
    real cont_prob = real(0.9);

    int worker_count   = 0;
    int task_grid_size = 32;

    bool use_mis = true;

    int spp = 1;
};

std::shared_ptr<Renderer> create_path_tracing_renderer(const PathTracingRendererParams &params);

// particle tracing

struct ParticleTracingRendererParams
{
    int worker_count = 0;

    // backward tracing

    int particle_task_count = 1;
    int particles_per_task = 1000;

    int min_depth = 5;
    int max_depth = 10;
    real cont_prob = real(0.9);

    // forward tracing

    int forward_task_grid_size = 32;
    int forward_spp = 1;
};

std::shared_ptr<Renderer> create_particle_tracing_renderer(const ParticleTracingRendererParams &params);

// ambient occlusion

struct AORendererParams
{
    int worker_count    = 0;
    int task_grid_size  = 32;
    int ao_sample_count = 5;

    Spectrum low_color  = Spectrum(0);
    Spectrum high_color = Spectrum(1);
    real max_occlusion_distance = 1;

    Spectrum background_color = Spectrum(0);

    int spp = 1;
};

std::shared_ptr<Renderer> create_ao_renderer(const AORendererParams &params);

// bidirectional path tracing

struct BDPTRendererParams
{
    int worker_count   = 0;
    int task_grid_size = 32;

    int cam_max_vtx_cnt = 10;
    int lht_max_vtx_cnt = 10;

    bool use_mis = true;

    int spp = 1;
};

std::shared_ptr<Renderer> create_bdpt_renderer(const BDPTRendererParams &params);

AGZ_TRACER_END
