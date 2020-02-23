#pragma once

#include <agz/editor/renderer/particle_renderer.h>

AGZ_EDITOR_BEGIN

class BDPTRenderer : public ParticleRenderer
{
public:

    struct Params
    {
        int worker_count   = -1;
        int task_grid_size = 32;

        int max_cam_depth = 5;
        int max_lht_depth = 5;

        bool use_mis = true;

        bool enable_preview = true;
    };

    BDPTRenderer(const Params &params, int width, int height, std::shared_ptr<const tracer::Scene> scene);

    ~BDPTRenderer();

protected:

    Spectrum fast_render_pixel(
        const tracer::Scene &scene, const tracer::Ray &ray, tracer::Sampler &sampler, tracer::Arena &arena) override;

    Spectrum render_pixel(
        const tracer::Scene &scene, const tracer::Ray &ray, tracer::Sampler &sampler,
        tracer::Arena &arena, tracer::FilmFilterApplier::FilmGridView<Spectrum> &particle_film, uint64_t *particle_count) override;

    uint64_t exec_render_task(
        Framebuffer::Task &task, tracer::Sampler &sampler,
        tracer::FilmFilterApplier::FilmGridView<Spectrum> &particle_film) override;

private:

    tracer::render::TraceParams preview_params_;
    tracer::render::BDPTParams  bdpt_params_;
};

AGZ_EDITOR_END
