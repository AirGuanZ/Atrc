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
    virtual void render(Scene &scene, ProgressReporter &reporter, Film *film) = 0;
};

AGZ_TRACER_END
