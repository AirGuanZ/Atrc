#include <agz/tracer/create/renderer.h>
#include <agz/tracer/render/path_tracing.h>

#include "./perpixel_renderer.h"

AGZ_TRACER_BEGIN

class PathTracingRenderer : public PerPixelRenderer
{
    render::TraceParams params_;

    render::Pixel(*eval_func_)(
        const render::TraceParams &, const Scene &,
        const Ray &, Sampler &, Arena &);

public:

    explicit PathTracingRenderer(const PathTracingRendererParams &params)
        : PerPixelRenderer(
            params.worker_count,
            params.task_grid_size, params.spp)
    {
        params_.min_depth = params.min_depth;
        params_.max_depth = params.max_depth;
        params_.cont_prob = params.cont_prob;

        if(params.use_mis)
            eval_func_ = &render::trace_std;
        else
            eval_func_ = &render::trace_nomis;
    }

protected:

    Pixel eval_pixel(
        const Scene &scene, const Ray &ray,
        Sampler &sampler, Arena &arena) const override
    {
        return eval_func_(params_, scene, ray, sampler, arena);
    }
};

RC<Renderer> create_path_tracing_renderer(
    const PathTracingRendererParams &params)
{
    return newRC<PathTracingRenderer>(params);
}

AGZ_TRACER_END
