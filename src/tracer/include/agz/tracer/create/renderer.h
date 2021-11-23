#pragma once

#include <agz/tracer/core/renderer.h>
#include <agz/tracer/core/sampler.h>

AGZ_TRACER_BEGIN

// path tracing

struct PTRendererParams
{
    int min_depth  = 5;
    int max_depth  = 10;
    real cont_prob = real(0.9);

    int worker_count   = 0;
    int task_grid_size = 32;

    bool use_mis = true;

    int spp = 1;

    int specular_depth = 20;
};

RC<Renderer> create_pt_renderer(
    const PTRendererParams &params);

// particle tracing

struct AdjointPTRendererParams
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

RC<Renderer> create_adjoint_pt_renderer(
    const AdjointPTRendererParams &params);

// ambient occlusion

struct AORendererParams
{
    int worker_count    = 0;
    int task_grid_size  = 32;
    int ao_sample_count = 5;

    FSpectrum low_color  = FSpectrum(0);
    FSpectrum high_color = FSpectrum(1);
    real max_occlusion_distance = 1;

    FSpectrum background_color = FSpectrum(0);

    int spp = 1;
};

RC<Renderer> create_ao_renderer(const AORendererParams &params);

// volumetric bidirectional path tracing

struct VolBDPTRendererParams
{
    int worker_count   = 0;
    int task_grid_size = 32;

    int cam_max_vtx_cnt = 10;
    int lht_max_vtx_cnt = 10;

    int spp = 1;

    bool use_mis = true;
};

RC<Renderer> create_vol_bdpt_renderer(const VolBDPTRendererParams &params);

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

    int grid_accel_resolution = 64;
};

RC<Renderer> create_sppm_renderer(const SPPMRendererParams &params);

// pssmlt pt

struct PSSMLTPTRendererParams
{
    int worker_count = 0;

    // about pt

    int min_depth = 5;
    int max_depth = 10;
    real cont_prob = real(0.9);

    bool use_mis = true;

    // about pssmlt

    int startup_sample_count = 100000;
    int mut_per_pixel = 100;

    real sigma = real(0.01);
    real large_step_prob = real(0.35);

    int chain_count = 1000;
};

RC<Renderer> create_pssmlt_pt_renderer(const PSSMLTPTRendererParams &params);

// restir

struct ReSTIRParams
{
    int worker_count = 0;

    int M                    = 32;
    int I                    = 2;
    int spatial_reuse_radius = 20;
    int spatial_reuse_count  = 5;
    
    int spp = 1;
};

RC<Renderer> create_restir_renderer(const ReSTIRParams &params);

// restir gi

struct ReSTIRGIParams
{
    int worker_count = 0;

    int I                    = 2;
    int spatial_reuse_radius = 20;
    int spatial_reuse_count  = 5;

    int spp = 1;

    int  min_depth      = 5;
    int  max_depth      = 10;
    int  specular_depth = 20;
    real cont_prob = real(0.9);
};

RC<Renderer> create_restir_gi_renderer(const ReSTIRGIParams &params);

AGZ_TRACER_END
