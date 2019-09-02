#pragma once

#include <agz/tracer/core/film.h>
#include <agz/tracer_utility/object.h>

AGZ_TRACER_BEGIN

class ProgressReporter;
class Scene;

/**
 * @brief 渲染器接口，负责将场景转换为图像
 */
class Renderer : public obj::Object
{
public:

    using Object::Object;

    virtual void render(Scene &scene, ProgressReporter &reporter, Film *film)
    {
        render_async(scene, reporter, film);
        join();
    }

    virtual void render_async(Scene&, ProgressReporter&, Film*)
    {
        throw std::runtime_error("Renderer::async rendering is unimplemented");
    }

    virtual void join()
    {
        throw std::runtime_error("Renderer::async rendering is unimplemented");
    }

    virtual void stop()
    {
        throw std::runtime_error("Renderer::async rendering is unimplemented");
    }
};

AGZT_INTERFACE(Renderer)

AGZ_TRACER_END
