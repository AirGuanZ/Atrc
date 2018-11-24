#pragma once

#include <Atrc/Core/Core.h>
#include <Atrc/Renderer/ParallelRenderer.h>

AGZ_NS_BEG(Atrc)

class SerialRenderer : public Renderer
{
    ParallelRenderer parallelRenderer;

public:

    SerialRenderer()
        : parallelRenderer(1)
    {
        
    }

    void Render(
        const SubareaRenderer &subareaRenderer, const Scene &scene, const Integrator &integrator,
        RenderTarget *rt, ProgressReporter *reporter) const override
    {
        parallelRenderer.Render(subareaRenderer, scene, integrator, rt, reporter);
    }
};

AGZ_NS_END(Atrc)
