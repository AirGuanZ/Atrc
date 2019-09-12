#include <iostream>

#include <agz/tracer/core/reporter.h>
#include <agz/utility/time.h>

AGZ_TRACER_BEGIN

class NoOutput : public ProgressReporter
{
    time::clock_t clock_;
    double total_seconds_ = 0;
    double last_stage_seconds_ = 0;

public:

    using ProgressReporter::ProgressReporter;

    static std::string description()
    {
        return R"___(
noout [ProgressReporter]
)___";
    }

    void progress(double percent) override
    {

    }

    void message(const std::string &msg) override
    {
        std::cout << "[MSG] " << msg << std::endl;
    }

    void error(const std::string &err) override
    {
        std::cout << "[ERR] " << err << std::endl;
    }

    void begin() override
    {
        total_seconds_ = 0;
    }

    void end() override
    {

    }

    void new_stage() override
    {
        clock_.restart();
    }

    void end_stage() override
    {
        last_stage_seconds_ = clock_.ms() / 1000.0;
        total_seconds_ += last_stage_seconds_;
    }

    double total_seconds() override
    {
        return total_seconds_;
    }

    double last_stage_seconds() override
    {
        return last_stage_seconds_;
    }
};

AGZT_IMPLEMENTATION(ProgressReporter, NoOutput, "noout")

AGZ_TRACER_END
