#pragma once

#include <atomic>

#include <agz/editor/renderer/framebuffer.h>
#include <agz/editor/renderer/renderer.h>

AGZ_EDITOR_BEGIN

struct AOParams
{
    Spectrum background_color;
    Spectrum low_color;
    Spectrum high_color;

    int worker_count = -1;
    int ao_sample_count = 4;
    real occlusion_distance = real(0.1);
};

class AO : public Renderer
{
public:

    AO(
        const AOParams &params, int fb_width, int fb_height,
        std::shared_ptr<const tracer::Scene> scene);

    ~AO();

    void start() override;

    Image2D<math::color3b> get_image() const override;

private:

    void exec_render_task(Framebuffer::Task &task, tracer::Sampler *sampler);

    std::atomic<bool> stop_rendering_;

    AOParams params_;

    std::shared_ptr<const tracer::Scene> scene_;

    Framebuffer framebuffer_;

    std::vector<std::thread> threads_;

    tracer::Arena sampler_arena_;
};

AGZ_EDITOR_END
