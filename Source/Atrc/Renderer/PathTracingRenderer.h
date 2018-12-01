#pragma once

#include <Atrc/Core/Core.h>
#include <Atrc/Renderer/PathTracingIntegrator/PathTracingIntegrator.h>
#include <Atrc/Utility/GridDivider.h>

AGZ_NS_BEG(Atrc)

class PathTracingRenderer : public Renderer
{
    using SubareaRect = GridDivider<uint32_t>::Grid;

    int workerCount_;
    uint32_t spp_;
    const PathTracingIntegrator &integrator_;

    void RenderSubarea(const Scene &scene, RenderTarget *rt, const SubareaRect &subarea) const;

public:

    PathTracingRenderer(int workerCount, uint32_t spp, const PathTracingIntegrator &integrator);

    void Render(const Scene &scene, RenderTarget *rt, ProgressReporter *reporter) const override;
};

AGZ_NS_END(Atrc)
