#pragma once

#include <mutex>

#include <AGZUtils/Utils/Thread.h>

#include <Atrc/Core/Core/Renderer.h>
#include <Atrc/Core/Utility/GridDivider.h>

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

    struct Task
    {
        Grid grid;
        int epoch;
    };

    int taskGridSize_;
    AGZ::StaticTaskDispatcher<Task> dispatcher_;

    std::mutex mergeMut_;
    size_t totalCount_;
    std::atomic<size_t> finishedCount_;

    size_t totalEpoch_;

    bool shuffle_;

    const PathTracingIntegrator &integrator_;

    void RenderGrid(const Scene *scene, FilmGrid *filmGrid, Sampler *sampler) const;

public:

    PathTracingRenderer(int workerCount, int taskGridSize, int epoch, bool shuffle, const PathTracingIntegrator &integrator) noexcept;

    void Render(const Scene *scene, Sampler *sampler, Film *film, Reporter *reporter) override;

    bool IsCompleted() const override;

    void Join(Reporter *reporter) override;
	
	void Stop() override;
};

} // namespace Atrc
