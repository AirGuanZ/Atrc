#include <agz/tracer/factory/creator/renderer_creators.h>
#include <agz/tracer/factory/raw/renderer.h>

AGZ_TRACER_FACTORY_BEGIN

namespace renderer
{

    class ParticleTracingRendererCreator : public Creator<Renderer>
    {
    public:

        std::string name() const override
        {
            return "particle";
        }

        std::shared_ptr<Renderer> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            ParticleTracingRendererParams renderer_params;

            renderer_params.worker_count               = params.child_int_or("worker_count", 0);

            renderer_params.particle_task_count        = params.child_int("particle_task_count");
            renderer_params.particle_sampler_prototype = context.create<Sampler>(params.child_group("backward_sampler"));
            renderer_params.min_depth                  = params.child_int_or("min_depth", 5);
            renderer_params.max_depth                  = params.child_int_or("max_depth", 10);
            renderer_params.cont_prob                  = params.child_real_or("cont_prob", real(0.9));

            renderer_params.forward_task_grid_size    = params.child_int_or("forward_task_grid_size", 32);
            renderer_params.forward_sampler_prototype = context.create<Sampler>(params.child_group("forward_sampler"));

            return create_particle_tracing_renderer(renderer_params);
        }
    };

    class PathTracingRendererCreator : public Creator<Renderer>
    {
    public:

        std::string name() const override
        {
            return "pt";
        }

        std::shared_ptr<Renderer> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            int worker_count   = params.child_int_or("worker_count", 0);
            int task_grid_size = params.child_int_or("task_grid_size", 32);
            auto sampler       = context.create<Sampler>(params.child_group("sampler"));
            auto integrator    = context.create<PathTracingIntegrator>(params.child_group("integrator"));

            PathTracingRendererParams pt_params;
            pt_params.worker_count      = worker_count;
            pt_params.task_grid_size    = task_grid_size;
            pt_params.sampler_prototype = sampler;
            pt_params.integrator        = integrator;

            return create_path_tracing_renderer(pt_params);
        }
    };

} // namespace renderer

void initialize_renderer_factory(Factory<Renderer> &factory)
{
    factory.add_creator(std::make_unique<renderer::ParticleTracingRendererCreator>());
    factory.add_creator(std::make_unique<renderer::PathTracingRendererCreator>());
}

AGZ_TRACER_FACTORY_END
