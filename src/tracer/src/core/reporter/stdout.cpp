#include <agz/tracer/core/renderer_interactor.h>
#include <agz/tracer/utility/logger.h>
#include <agz/utility/console.h>

AGZ_TRACER_BEGIN

class StdOutput : public RendererInteractor
{
    double percent_ = 0;

    console::progress_bar_f_t pbar_ = console::progress_bar_f_t(80, '=');

public:

    bool need_image_preview() const noexcept override
    {
        return false;
    }

    void progress(double percent, const PreviewFunc &get_image_preview) override
    {
        if(percent > percent_)
        {
            pbar_.set_percent(static_cast<float>(percent));
            pbar_.display();
            percent_ = percent;
        }
    }

    void message(const std::string &msg) override
    {
        AGZ_INFO(msg);
    }

    void begin() override
    {

    }

    void end() override
    {

    }

    void new_stage() override
    {
        percent_ = 0;
        pbar_.reset_time();
        pbar_.set_percent(0);
        pbar_.display();
    }

    void end_stage() override
    {
        pbar_.done();
    }
};

RC<RendererInteractor> create_stdout_reporter()
{
    return newRC<StdOutput>();
}

AGZ_TRACER_END
