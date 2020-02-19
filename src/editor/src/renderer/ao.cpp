#include <agz/editor/renderer/ao.h>
#include <agz/tracer/tracer.h>
#include <agz/utility/thread.h>

AGZ_EDITOR_BEGIN

AO::AO(const Params &params, int fb_width, int fb_height, std::shared_ptr<const tracer::Scene> scene)
    : PerPixelRenderer(
        params.worker_count, params.task_grid_size, 2,
        fb_width, fb_height, params.enable_preview, 128, 32, scene)
{
    ao_params_.ao_sample_count        = params.ao_sps;
    ao_params_.max_occlusion_distance = params.occlusion_distance;
    ao_params_.background_color       = params.background_color;
    ao_params_.low_color              = params.low_color;
    ao_params_.high_color             = params.high_color;

    fast_params_ = ao_params_;
    fast_params_.ao_sample_count = 4;
}

AO::~AO()
{
    stop_rendering();
}

Spectrum AO::fast_render_pixel(const tracer::Scene &scene, const tracer::Ray &ray, tracer::Sampler &sampler, tracer::Arena &arena)
{
    return trace_ao(ao_params_, scene, ray, sampler).value;
}

Spectrum AO::render_pixel(const tracer::Scene &scene, const tracer::Ray &ray, tracer::Sampler &sampler, tracer::Arena &arena)
{
    return trace_ao(fast_params_, scene, ray, sampler).value;
}

AGZ_EDITOR_END
