#pragma once

#include <atomic>
#include <future>

#include <agz/tracer/core/render_target.h>

AGZ_TRACER_BEGIN

class RendererInteractor;
class Scene;

/**
 * @brief rendering algorithm interface
 */
class Renderer
{
protected:

    std::atomic<bool> stop_rendering_ = false;
    std::atomic<bool> doing_rendering_ = false;

    bool is_waitable_ = false;
    std::future<RenderTarget> async_thread_;

public:

    virtual ~Renderer() { stop_async(); }

    /**
     * @brief blocking rendering
     */
    virtual RenderTarget render(
        FilmFilterApplier filter, Scene &scene,
        RendererInteractor &reporter) = 0;

    /**
     * @brief start the async rendering
     */
    void render_async(
        FilmFilterApplier filter, Scene &scene,
        RendererInteractor &reporter);

    /**
     * @brief stop the async rendering
     */
    void stop_async();

    /**
     * @brief wait the async rendering to complete
     */
    RenderTarget wait_async();

    /**
     * @brief returns true after calling render_async and
     *  before calling stop_async/wait_async
     */
    bool is_waitable() const noexcept { return is_waitable_; }

    /**
     * @brief is the async rendering actually completed
     */
    bool is_doing_rendering() const noexcept { return doing_rendering_; }
};

AGZ_TRACER_END
