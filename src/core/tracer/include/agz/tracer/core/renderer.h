#pragma once

#include <agz/tracer/core/film.h>

AGZ_TRACER_BEGIN

class ProgressReporter;
class Scene;

/**
 * @brief 渲染器接口，负责将场景转换为图像
 */
class Renderer
{
public:

    virtual ~Renderer() = default;

    /**
     * @brief 阻塞式渲染
     */
    virtual void render(Scene &scene, ProgressReporter &reporter, Film *film)
    {
        render_async(scene, reporter, film);
        join();
    }

    /**
     * @brief 开始异步渲染
     */
    virtual void render_async(Scene&, ProgressReporter&, Film*)
    {
        throw std::runtime_error("Renderer::async rendering is unimplemented");
    }

    /**
     * @brief 等待异步渲染结束
     */
    virtual void join()
    {
        throw std::runtime_error("Renderer::async rendering is unimplemented");
    }

    /**
     * @brief 强制结束异步渲染
     */
    virtual void stop()
    {
        throw std::runtime_error("Renderer::async rendering is unimplemented");
    }
};

AGZ_TRACER_END
