#pragma once

#include <Atrc/Core/Core.h>

AGZ_NS_BEG(Atrc)

class ParallelRenderer : public Renderer
{
    int workerCount_;

public:

    explicit ParallelRenderer(int workerCount = -1);

    void Render(
        const SubareaRenderer &subareaRenderer, const Scene &scene, const Integrator &integrator,
        RenderTarget *output, ProgressReporter *reporter) const override;
};

AGZ_NS_END(Atrc)
