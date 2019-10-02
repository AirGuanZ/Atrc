#include <agz/tracer/factory/creator/reporter_creators.h>
#include <agz/tracer/factory/raw/reporter.h>

AGZ_TRACER_FACTORY_BEGIN

namespace reporter
{
    
    class StdOutCreator : public Creator<ProgressReporter>
    {
    public:

        std::string name() const override
        {
            return "stdout";
        }

        std::shared_ptr<ProgressReporter> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            return create_stdout_reporter();
        }
    };

    class NoOutCreator : public Creator<ProgressReporter>
    {
    public:

        std::string name() const override
        {
            return "noout";
        }

        std::shared_ptr<ProgressReporter> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            return create_noout_reporter();
        }
    };

} // namespace reporter

void initialize_reporter_factory(Factory<ProgressReporter> &factory)
{
    factory.add_creator(std::make_unique<reporter::StdOutCreator>());
    factory.add_creator(std::make_unique<reporter::NoOutCreator>());
}

AGZ_TRACER_FACTORY_END
