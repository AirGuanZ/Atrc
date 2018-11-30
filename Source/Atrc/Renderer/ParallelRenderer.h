#pragma once

#include <Atrc/Core/Core.h>

AGZ_NS_BEG(Atrc)

class ParallelRenderer : public Renderer
{
    using SubareaRect = GridDivider<uint32_t>::Grid;

    int workerCount_;
    uint32_t spp_;
    const Integrator &integrator_;

    void RenderSubarea(const Scene &scene, RenderTarget *rt, const SubareaRect &subarea) const;

public:

    ParallelRenderer(int workerCount, uint32_t spp, const Integrator &integrator);

    void Render(const Scene &scene, RenderTarget *rt, ProgressReporter *reporter) const override;
};

AGZ_NS_END(Atrc)
