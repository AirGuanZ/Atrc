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

RC<Renderer> create_path_tracing_renderer(
    const PathTracingRendererParams &params);

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

RC<Renderer> create_particle_tracing_renderer(
    const ParticleTracingRendererParams &params);

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

RC<Renderer> create_ao_renderer(const AORendererParams &params);

// bidirectional path tracing

struct BDPTRendererParams
{
    int worker_count   = 0;
    int task_grid_size = 32;

    int cam_max_vtx_cnt = 10;
    int lht_max_vtx_cnt = 10;

    int spp = 1;
};

RC<Renderer> create_bdpt_renderer(const BDPTRendererParams &params);

// sppm

struct SPPMRendererParams
{
    int worker_count           = 0;

    int forward_task_grid_size = 64;
    int forward_max_depth      = 8;

    real init_radius = real(0.1);

    int iteration_count       = 100;
    int photons_per_iteration = 100000;

    int photon_min_depth = 5;
    int photon_max_depth = 10;
    real photon_cont_prob = real(0.9);

    real update_alpha = real(2) / 3;

    int grid_accel_resolution  = 128;
};

RC<Renderer> create_sppm_renderer(const SPPMRendererParams &params);

AGZ_TRACER_END
