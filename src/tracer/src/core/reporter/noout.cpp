#include <iostream>

#include <agz/tracer/core/renderer_interactor.h>
#include <agz/utility/misc.h>

AGZ_TRACER_BEGIN

class NoOutput : public RendererInteractor
{
public:

    bool need_image_preview() const noexcept override
    {
        return false;
    }

    void progress(double percent, const PreviewFunc &get_image_preview) override
    {

    }

    void message(const std::string &msg) override
    {
        std::cout << "[MSG] " << msg << std::endl;
    }

    void begin() override
    {

    }

    void end() override
    {

    }

    void new_stage() override
    {

    }

    void end_stage() override
    {
        
    }
};

RC<RendererInteractor> create_noout_reporter()
{
    return newRC<NoOutput>();
}

AGZ_TRACER_END
