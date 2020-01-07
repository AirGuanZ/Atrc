#pragma once

#include <agz/tracer/core/render_target.h>

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
    virtual RenderTarget render(const FilmFilterApplier &filter, Scene &scene, ProgressReporter &reporter) = 0;
};

AGZ_TRACER_END
