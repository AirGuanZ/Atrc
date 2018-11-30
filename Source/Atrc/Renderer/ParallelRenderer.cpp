#include <vector>

#include "ParallelRenderer.h"

AGZ_NS_BEG(Atrc)

ParallelRenderer::ParallelRenderer(int workerCount)
    : workerCount_(workerCount)
{

}

void ParallelRenderer::Render(
    const SubareaRenderer &subareaRenderer, const Scene &scene, const Integrator &integrator,
    RenderTarget *output, ProgressReporter *reporter) const
{
    AGZ_ASSERT(output->IsAvailable());

    std::queue<SubareaRect> tasks = GridDivider<uint32_t>::Divide(
        { 0, output->GetWidth(), 0, output->GetHeight() }, 16, 16);

    if(reporter)
        reporter->Begin();

    std::atomic<size_t> finishedCount = 0;
    size_t totalCount = tasks.size();

    auto func = [&](const SubareaRect &subarea, AGZ::NoSharedParam_t)
    {
        subareaRenderer.Render(scene, integrator, *output, subarea);
        auto percent = 100.0 * ++finishedCount / totalCount;
        reporter->Report(percent);
    };

    AGZ::StaticTaskDispatcher<SubareaRect, AGZ::NoSharedParam_t> dispatcher(workerCount_);

    if(dispatcher.Run(func, AGZ::NO_SHARED_PARAM, tasks))
    {
        if(reporter)
            reporter->End();
    }
    else if(reporter)
    {
        for(auto &err : dispatcher.GetExceptions())
            reporter->Message(err.what());
    }
}

AGZ_NS_END(Atrc)
