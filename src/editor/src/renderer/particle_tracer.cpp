#include <agz/editor/renderer/particle_tracer.h>

AGZ_EDITOR_BEGIN

ParticleTracer::ParticleTracer(
    const Params &params, int width, int height, std::shared_ptr<const tracer::Scene> scene)
    : ParticleRenderer(params.worker_count, params.task_grid_size, 3,
                       width, height, params.enable_preview, 128, 32, scene)
{
    particle_sample_count_ = params.particle_sample_count;

    preview_params_.min_depth = params.min_depth;
    preview_params_.max_depth = params.max_depth;
    preview_params_.cont_prob = params.cont_prob;
    preview_params_.direct_illum_sample_count = 1;

    trace_params_.min_depth = params.min_depth;
    trace_params_.max_depth = params.max_depth;
    trace_params_.cont_prob = params.cont_prob;
    trace_params_.film_res  = { real(width), real(height) };
}

ParticleTracer::~ParticleTracer()
{
    stop_rendering();
}

Spectrum ParticleTracer::fast_render_pixel(
    const tracer::Scene &scene, const tracer::Ray &ray, tracer::Sampler &sampler, tracer::Arena &arena)
{
    return trace_std(preview_params_, scene, ray, sampler, arena).value;
}

Spectrum ParticleTracer::render_pixel(
    const tracer::Scene &scene, const tracer::Ray &ray, tracer::Sampler &sampler,
    tracer::Arena &arena, tracer::FilmFilterApplier::FilmGridView<Spectrum> &particle_film, uint64_t *particle_count)
{
    using namespace tracer;

    for(int i = 0; i < particle_sample_count_; ++i)
        trace_particle(trace_params_, scene, sampler, particle_film, arena);
    *particle_count += particle_sample_count_;

    EntityIntersection inct;
    if(!scene.closest_intersection(ray, &inct))
    {
        if(auto env = scene.envir_light())
            return env->radiance(ray.o, ray.d);
        return Spectrum();
    }

    if(auto light = inct.entity->as_light())
        return light->radiance(inct.pos, inct.geometry_coord.z, -ray.d);
    return Spectrum();
}

AGZ_EDITOR_END
