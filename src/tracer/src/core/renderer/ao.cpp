#include <mutex>
#include <thread>

#include <agz/tracer/core/camera.h>
#include <agz/tracer/core/renderer.h>
#include <agz/tracer/core/renderer_interactor.h>
#include <agz/tracer/core/sampler.h>
#include <agz/tracer/core/scene.h>
#include <agz/tracer/create/renderer.h>
#include <agz/tracer/render/path_tracing.h>
#include <agz-utils/thread.h>

#include "./perpixel_renderer.h"

AGZ_TRACER_BEGIN

class AORenderer : public PerPixelRenderer
{
    render::AOParams params_;

public:

    explicit AORenderer(const AORendererParams &params)
        : PerPixelRenderer(
            params.worker_count, params.task_grid_size, params.spp)
    {
        params_.background_color       = params.background_color;
        params_.low_color              = params.low_color;
        params_.high_color             = params.high_color;
        params_.ao_sample_count        = params.ao_sample_count;
        params_.max_occlusion_distance = params.max_occlusion_distance;
    }

protected:

    Pixel eval_pixel(
        const Scene &scene, const Ray &ray,
        Sampler &sampler, Arena &arena) const override
    {
        return trace_ao(params_, scene, ray, sampler);
    }
};

RC<Renderer> create_ao_renderer(const AORendererParams &params)
{
    return newRC<AORenderer>(params);
}

AGZ_TRACER_END
