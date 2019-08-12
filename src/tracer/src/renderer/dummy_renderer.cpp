#include <agz/tracer/core/renderer.h>

AGZ_TRACER_BEGIN

class DummyRenderer : public Renderer
{
public:

    using Renderer::Renderer;

    static std::string description()
    {
        return R"___(
dummy [Renderer]
    this renderer performs no rendering at all
)___";
    }

    void render_async(Scene&, ProgressReporter&, Film*) override
    {
        
    }

    void join() override
    {
        
    }

    void stop() override
    {
        
    }
};

AGZT_IMPLEMENTATION(Renderer, DummyRenderer, "dummy")

AGZ_TRACER_END
