#include <agz/tracer/factory/creator/path_tracing_integrator_creators.h>
#include <agz/tracer/factory/raw/path_tracing_integrator.h>

AGZ_TRACER_FACTORY_BEGIN

namespace pt_integrator
{
    
    class MISVolCreator : public Creator<PathTracingIntegrator>
    {
    public:

        std::string name() const override
        {
            return "mis";
        }

        std::shared_ptr<PathTracingIntegrator> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            int min_depth = params.child_int_or("min_depth", 5);
            int max_depth = params.child_int_or("max_depth", 10);
            real cont_prob = params.child_real_or("cont_prob", real(0.9));
            return create_mis_integrator(min_depth, max_depth, cont_prob);
        }
    };

    class NativeVolCreator : public Creator<PathTracingIntegrator>
    {
    public:

        std::string name() const override
        {
            return "native";
        }

        std::shared_ptr<PathTracingIntegrator> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            int min_depth = params.child_int_or("min_depth", 5);
            int max_depth = params.child_int_or("max_depth", 10);
            real cont_prob = params.child_real_or("cont_prob", real(0.9));
            return create_native_integrator(min_depth, max_depth, cont_prob);
        }
    };

} // namespace pt_integrator

void initialize_path_tracing_integrator_factory(Factory<PathTracingIntegrator> &factory)
{
    factory.add_creator(std::make_unique<pt_integrator::MISVolCreator>());
    factory.add_creator(std::make_unique<pt_integrator::NativeVolCreator>());
}

AGZ_TRACER_FACTORY_END
