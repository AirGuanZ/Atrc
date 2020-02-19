#pragma once

#include <agz/editor/renderer/framebuffer.h>
#include <agz/editor/renderer/renderer.h>

AGZ_EDITOR_BEGIN

class AO : public PerPixelRenderer
{
public:

    struct Params
    {
        int worker_count   = -1;
        int task_grid_size = 32;

        int ao_sps = 4;
        real occlusion_distance = real(1);

        Spectrum background_color;
        Spectrum low_color;
        Spectrum high_color;

        bool enable_preview = true;
    };

    AO(const Params &params, int fb_width, int fb_height, std::shared_ptr<const tracer::Scene> scene);

    ~AO();

protected:

    Spectrum fast_render_pixel(const tracer::Scene &scene, const tracer::Ray &ray, tracer::Sampler &sampler, tracer::Arena &arena) override;

    Spectrum render_pixel(const tracer::Scene &scene, const tracer::Ray &ray, tracer::Sampler &sampler, tracer::Arena &arena) override;

private:

    tracer::render::AOParams ao_params_;
    tracer::render::AOParams fast_params_;
};

AGZ_EDITOR_END
