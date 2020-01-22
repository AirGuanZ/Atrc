#include <agz/tracer/core/renderer.h>

AGZ_TRACER_BEGIN

void Renderer::render_async(FilmFilterApplier filter, Scene &scene, ProgressReporter &reporter)
{
    stop_rendering_ = false;
    async_thread_ = std::async(std::launch::async, [this, filter, &scene, &reporter]()
    {
        return this->render(std::move(filter), scene, reporter);
    });
    is_async_rendering_ = true;
}

void Renderer::stop_async()
{
    if(is_async_rendering_)
    {
        stop_rendering_ = true;
        if(async_thread_.valid())
            async_thread_.wait();
        stop_rendering_ = false;

        is_async_rendering_ = false;
    }
}

RenderTarget Renderer::sync()
{
    assert(is_async_rendering_);
    AGZ_SCOPE_GUARD({ is_async_rendering_ = false; });
    return async_thread_.get();
}

AGZ_TRACER_END
