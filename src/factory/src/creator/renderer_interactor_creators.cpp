#include <agz/factory/creator/renderer_interactor_creators.h>
#include <agz/tracer/create/renderer_interactor.h>

AGZ_TRACER_FACTORY_BEGIN

namespace reporter
{
    
    class StdOutCreator : public Creator<RendererInteractor>
    {
    public:

        std::string name() const override
        {
            return "stdout";
        }

        std::shared_ptr<RendererInteractor> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            return create_stdout_reporter();
        }
    };

    class NoOutCreator : public Creator<RendererInteractor>
    {
    public:

        std::string name() const override
        {
            return "noout";
        }

        std::shared_ptr<RendererInteractor> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            return create_noout_reporter();
        }
    };

} // namespace reporter

void initialize_renderer_interactor_factory(Factory<RendererInteractor> &factory)
{
    factory.add_creator(std::make_unique<reporter::StdOutCreator>());
    factory.add_creator(std::make_unique<reporter::NoOutCreator>());
}

AGZ_TRACER_FACTORY_END
