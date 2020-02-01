#pragma once

#include <atomic>

#include <agz/editor/renderer/framebuffer.h>
#include <agz/editor/renderer/renderer.h>

AGZ_EDITOR_BEGIN

struct PathTracingParams
{
    int worker_count = -1;

    int min_depth = 3;
    int max_depth = 10;
    real cont_prob = real(0.8);
};

class PathTracer : public Renderer
{
public:

    PathTracer(
        const PathTracingParams &params, int fb_width, int fb_height,
        std::shared_ptr<const tracer::Scene> scene);

    ~PathTracer();

    void start() override;

    Image2D<math::color3b> get_image() const override;

private:

    void exec_render_task(Framebuffer::Task &task, tracer::Sampler *sampler);

    std::atomic<bool> stop_rendering_;

    PathTracingParams params_;

    std::shared_ptr<const tracer::Scene> scene_;

    Framebuffer framebuffer_;

    std::vector<std::thread> threads_;

    tracer::Arena sampler_arena_;
};

AGZ_EDITOR_END
