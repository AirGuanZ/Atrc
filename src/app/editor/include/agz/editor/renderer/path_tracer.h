#pragma once

#include <agz/editor/renderer/framebuffer.h>
#include <agz/editor/renderer/renderer.h>

AGZ_EDITOR_BEGIN

class PathTracer : public PerPixelRenderer
{
public:

    struct Params
    {
        int worker_count   = -1;
        int task_grid_size = 32;

        int min_depth  = 5;
        int max_depth  = 10;
        real cont_prob = real(0.9);

        bool fast_preview = false;
    };

    PathTracer(const Params &params, int fb_width, int fb_height, std::shared_ptr<const tracer::Scene> scene);

    ~PathTracer();

protected:

    Spectrum fast_render_pixel(const tracer::Scene &scene, const tracer::Ray &ray, tracer::Sampler &sampler, tracer::Arena &arena) override;

    Spectrum render_pixel(const tracer::Scene &scene, const tracer::Ray &ray, tracer::Sampler &sampler, tracer::Arena &arena) override;

private:

    bool fast_preview_;
    tracer::render::TraceParams    trace_params_;
    tracer::render::AlbedoAOParams fast_params_;
};

AGZ_EDITOR_END
