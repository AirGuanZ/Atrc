#pragma once

#include <Atrc/Lib/Core/Renderer.h>
#include <Atrc/Lib/Utility/GridDivider.h>

namespace Atrc
{

class PathTracingIntegrator
{
public:

    virtual ~PathTracingIntegrator() = default;

    virtual Spectrum Eval(const Scene &scene, const Ray &r, Sampler *sampler, Arena &arena) const = 0;
};

class PathTracingRenderer : public Renderer
{
    using Grid = GridDivider<int32_t>::Grid;

    int workerCount_;
    int taskGridSize_;

    const PathTracingIntegrator &integrator_;

    void RenderGrid(const Scene &scene, FilmGrid *filmGrid, Sampler *sampler) const;

public:

    PathTracingRenderer(int workerCount, int taskGridSize, const PathTracingIntegrator &integrator) noexcept;

    void Render(const Scene &scene, Sampler *sampler, Film *film) const override;
};

} // namespace Atrc
