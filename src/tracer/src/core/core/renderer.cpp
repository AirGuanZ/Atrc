#include <agz/tracer/core/renderer.h>

AGZ_TRACER_BEGIN

void Renderer::render_async(
    FilmFilterApplier filter, Scene &scene, RendererInteractor &reporter)
{
    stop_rendering_ = false;
    doing_rendering_ = true;
    async_thread_ = std::async(
        std::launch::async, [this, filter, &scene, &reporter]()
    {
        AGZ_SCOPE_GUARD({ doing_rendering_ = false; });
        return this->render(std::move(filter), scene, reporter);
    });
    is_waitable_ = true;
}

void Renderer::stop_async()
{
    if(is_waitable_)
    {
        stop_rendering_ = true;
        if(async_thread_.valid())
            async_thread_.wait();
        stop_rendering_ = false;
        is_waitable_ = false;
    }
}

RenderTarget Renderer::wait_async()
{
    assert(is_waitable_);
    AGZ_SCOPE_GUARD({ is_waitable_ = false; });
    return async_thread_.get();
}

AGZ_TRACER_END
