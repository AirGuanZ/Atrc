#pragma once

#include <agz/editor/renderer/renderer.h>

AGZ_EDITOR_BEGIN

class PerPixelRenderer : public Renderer
{
public:

    PerPixelRenderer(
        int worker_count, int task_grid_size, int init_pixel_size,
        int framebuffer_width, int framebuffer_height,
        bool enable_fast_rendering, int fast_resolution, int fast_task_grid_size,
        RC<const tracer::Scene> scene);

    ~PerPixelRenderer();

    Image2D<Spectrum> start() override;

    Image2D<Spectrum> get_image() const override;

protected:

    void stop_rendering();

    virtual Spectrum fast_render_pixel(
        const tracer::Scene &scene, const tracer::Ray &ray,
        tracer::Sampler &sampler, tracer::Arena &arena) = 0;

    virtual Spectrum render_pixel(
        const tracer::Scene &scene, const tracer::Ray &ray,
        tracer::Sampler &sampler, tracer::Arena &arena) = 0;

private:

    Image2D<Spectrum> do_fast_rendering();

    void exec_fast_render_task(
        Image2D<Spectrum> &target, const Vec2i &beg, const Vec2i &end,
        tracer::Sampler &sampler);

    void exec_render_task(Framebuffer::Task &task, tracer::Sampler *sampler);

    int worker_count_;

    int framebuffer_width_;
    int framebuffer_height_;

    bool enable_fast_rendering_;
    int fast_resolution_;
    int fast_task_grid_size_;

    RC<const tracer::Scene> scene_;

    Framebuffer framebuffer_;

    std::atomic<bool> stop_rendering_;
    std::vector<std::thread> threads_;

    tracer::Arena sampler_arena_;
};

AGZ_EDITOR_END
