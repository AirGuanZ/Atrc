#pragma once

#include <Atrc/Lib/Core/Renderer.h>
#include <Atrc/Lib/Utility/GridDivider.h>

namespace Atrc
{

class PathTracingIntegrator
{
public:

    virtual ~PathTracingIntegrator() = default;

    virtual Spectrum Eval(const Scene &scene, const Ray &r, Arena &arena) const = 0;
};

class PathTracingRenderer : public Renderer
{
    using SubareaRect = GridDivider<uint32_t>::Grid;

    int workerCount_;
    uint32_t spp_;

    uint32_t taskGridSize_;

    const PathTracingIntegrator &integrator_;

    void RenderSubarea(const Scene &scene, RenderTarget *rt, const SubareaRect &subarea) const;

public:

    PathTracingRenderer(int workerCount, uint32_t spp, uint32_t taskGridSize, const PathTracingIntegrator &integrator);

    void Render(const Scene &scene, RenderTarget *rt) const override;
};

} // namespace Atrc
