#pragma once

#include <QObject>

#include <agz/editor/renderer/framebuffer.h>

AGZ_EDITOR_BEGIN

/**
 * @brief interactive path tracer
 *
 * all public methods (exclude destructor) must be thread safe
 */
class Renderer : public QObject, public misc::uncopyable_t
{
public:

    virtual ~Renderer() = default;

    /**
     * @brief called before start rendering
     *
     * @return a quickly rendered image for previewing
     */
    virtual Image2D<math::color3b> start() = 0;

    /**
     * @brief get current rendered image
     */
    virtual Image2D<math::color3b> get_image() const = 0;
};

class PerPixelRenderer : public Renderer
{
public:

    PerPixelRenderer(
        int worker_count, int task_grid_size, int init_pixel_size,
        int framebuffer_width, int framebuffer_height,
        bool enable_fast_rendering, int fast_resolution, int fast_task_grid_size,
        std::shared_ptr<const tracer::Scene> scene);

    ~PerPixelRenderer();

    Image2D<math::color3b> start() override;

    Image2D<math::color3b> get_image() const override;

protected:

    void stop_rendering();

    virtual Spectrum fast_render_pixel(const tracer::Scene &scene, const tracer::Ray &ray, tracer::Sampler &sampler, tracer::Arena &arena) = 0;

    virtual Spectrum render_pixel(const tracer::Scene &scene, const tracer::Ray &ray, tracer::Sampler &sampler, tracer::Arena &arena) = 0;

private:

    Image2D<math::color3b> do_fast_rendering();

    void exec_fast_render_task(Image2D<math::color3b> &target, const Vec2i &beg, const Vec2i &end, tracer::Sampler &sampler);

    void exec_render_task(Framebuffer::Task &task, tracer::Sampler *sampler);

    int worker_count_;

    int framebuffer_width_;
    int framebuffer_height_;

    bool enable_fast_rendering_;
    int fast_resolution_;
    int fast_task_grid_size_;

    std::shared_ptr<const tracer::Scene> scene_;

    Framebuffer framebuffer_;

    std::atomic<bool> stop_rendering_;
    std::vector<std::thread> threads_;

    tracer::Arena sampler_arena_;
};

AGZ_EDITOR_END
