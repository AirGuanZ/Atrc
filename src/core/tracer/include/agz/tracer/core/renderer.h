#pragma once

#include <atomic>
#include <future>

#include <agz/tracer/core/render_target.h>

AGZ_TRACER_BEGIN

class ProgressReporter;
class Scene;

/**
 * @brief 渲染器接口，负责将场景转换为图像
 */
class Renderer
{
protected:

    std::atomic<bool> stop_rendering_ = false;
    std::atomic<bool> doing_rendering_ = false;

    bool is_async_rendering_ = false;
    std::future<RenderTarget> async_thread_;

public:

    virtual ~Renderer() { stop_async(); }

    /**
     * @brief 阻塞式渲染
     */
    virtual RenderTarget render(FilmFilterApplier filter, Scene &scene, ProgressReporter &reporter) = 0;

    /**
     * @brief 发起异步渲染任务
     */
    void render_async(FilmFilterApplier filter, Scene &scene, ProgressReporter &reporter);

    /**
     * @brief 终止异步渲染任务
     */
    void stop_async();

    /**
     * @brief 等待异步渲染完成
     */
    RenderTarget wait_async();

    /**
     * @brief 是否正在异步渲染中
     */
    bool is_async_rendering() const noexcept { return is_async_rendering_; }

    /**
     * @brief 异步渲染实质上是否已经完成
     *
     * 即在调用render_async之后，即使未调用stop_async或wait_async，渲染结束后也会返回false
     */
    bool is_doing_rendering() const noexcept { return doing_rendering_; }
};

AGZ_TRACER_END
