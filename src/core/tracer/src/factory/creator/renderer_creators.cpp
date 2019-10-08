#include <agz/tracer/factory/creator/renderer_creators.h>
#include <agz/tracer/factory/raw/renderer.h>

AGZ_TRACER_FACTORY_BEGIN

namespace renderer
{

    class PathTracingRendererCreator : public Creator<Renderer>
    {
    public:

        std::string name() const override
        {
            return "pt";
        }

        std::shared_ptr<Renderer> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            int worker_count = params.child_int_or("worker_count", 0);
            auto sampler = context.create<Sampler>(params.child_group("sampler"));
            auto integrator = context.create<PathTracingIntegrator>(params.child_group("integrator"));

            PathTracingRendererParams pt_params;
            pt_params.worker_count = worker_count;
            pt_params.sampler_prototype = sampler;
            pt_params.integrator = integrator;

            return create_path_tracing_renderer(pt_params);
        }
    };

} // namespace renderer

void initialize_renderer_factory(Factory<Renderer> &factory)
{
    factory.add_creator(std::make_unique<renderer::PathTracingRendererCreator>());
}

AGZ_TRACER_FACTORY_END
