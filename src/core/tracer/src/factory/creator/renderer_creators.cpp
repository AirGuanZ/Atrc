#include <agz/tracer/factory/creator/renderer_creators.h>
#include <agz/tracer/factory/raw/renderer.h>

AGZ_TRACER_FACTORY_BEGIN

namespace renderer
{
    
    class PathTracerCreator : public Creator<Renderer>
    {
    public:

        std::string name() const override
        {
            return "pt";
        }

        std::shared_ptr<Renderer> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            auto integrator = context.create<PathTracingIntegrator>(params.child_group("integrator"));
            int worker_count = params.child_int("worker_count");
            int task_grid_size = params.child_int_or("task_grid_size", 32);
            auto sampler = context.create<Sampler>(params.child_group("sampler"));
            return create_path_tracer(std::move(integrator), worker_count, task_grid_size, std::move(sampler));
        }
    };

    class IsolatedRendererCreator : public Creator<Renderer>
    {
    public:

        std::string name() const override
        {
            return "isolated";
        }

        std::shared_ptr<Renderer> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            int min_depth = params.child_int_or("min_depth", 5);
            int max_depth = params.child_int_or("max_depth", 10);
            real cont_prob = params.child_real_or("cont_prob", real(0.9));

            int shading_aa = params.child_int_or("shading_aa", 1);
            int background_aa = params.child_int_or("background_aa", 1);

            Spectrum background_color = params.child_spectrum_or("background_color", Spectrum(1));
            bool hide_background_entity = params.child_int_or("hide_background_entity", 0) != 0;
            
            int worker_count = params.child_int("worker_count");
            int task_grid_size = params.child_int_or("task_grid_size", 32);
            auto sampler = context.create<Sampler>(params.child_group("sampler"));

            IsolatedPathTracerParams isolated_params;
            isolated_params.min_depth              = min_depth;
            isolated_params.max_depth              = max_depth;
            isolated_params.cont_prob              = cont_prob;
            isolated_params.shading_aa             = shading_aa;
            isolated_params.background_aa          = background_aa;
            isolated_params.background_color       = background_color;
            isolated_params.hide_background_entity = hide_background_entity;
            isolated_params.worker_count           = worker_count;
            isolated_params.task_grid_size         = task_grid_size;
            isolated_params.sampler_prototype      = sampler;

            return create_isolated_path_tracer(isolated_params);
        }
    };

} // namespace renderer

void initialize_renderer_factory(Factory<Renderer> &factory)
{
    factory.add_creator(std::make_unique<renderer::PathTracerCreator>());
    factory.add_creator(std::make_unique<renderer::IsolatedRendererCreator>());
}

AGZ_TRACER_FACTORY_END
