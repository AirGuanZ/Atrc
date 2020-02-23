#pragma once

#include <agz/editor/renderer/particle_renderer.h>

AGZ_EDITOR_BEGIN

class ParticleTracer : public ParticleRenderer
{
public:
    
    struct Params
    {
        int worker_count   = -1;
        int task_grid_size = 32;

        int particle_sample_count = 4;

        int min_depth  = 5;
        int max_depth  = 10;
        real cont_prob = real(0.9);

        bool enable_preview = true;
    };

    ParticleTracer(const Params &params, int width, int height, std::shared_ptr<const tracer::Scene> scene);

    ~ParticleTracer();

protected:

    Spectrum fast_render_pixel(
        const tracer::Scene &scene, const tracer::Ray &ray, tracer::Sampler &sampler, tracer::Arena &arena) override;

    Spectrum render_pixel(
        const tracer::Scene &scene, const tracer::Ray &ray, tracer::Sampler &sampler,
        tracer::Arena &arena, tracer::FilmFilterApplier::FilmGridView<Spectrum> &particle_film, uint64_t *particle_count) override;

private:

    int particle_sample_count_;

    tracer::render::TraceParams preview_params_;
    tracer::render::ParticleTraceParams trace_params_;
};

AGZ_EDITOR_END
