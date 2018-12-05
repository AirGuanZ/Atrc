#include <vector>

#include "PathTracingRenderer.h"

AGZ_NS_BEG(Atrc)

void PathTracingRenderer::RenderSubarea(const Scene &scene, RenderTarget *rt, const SubareaRect &subarea) const
{
    AGZ::ObjArena<> arena;

    auto cam = scene.GetCamera();
    for(uint32_t py = subarea.yBegin; py < subarea.yEnd; ++py)
    {
        for(uint32_t px = subarea.xBegin; px < subarea.xEnd; ++px)
        {
            Spectrum pixel = SPECTRUM::BLACK;
            for(uint32_t i = 0; i < spp_; ++i)
            {
                Real xOffset = Rand(), yOffset = Rand();
                auto [r, we, pdf] = cam->GetRay({ px + xOffset, py + yOffset });
                pixel += (we / float(pdf)) * integrator_.Eval(scene, r, arena);
            }
            rt->At(px, py) = pixel / spp_;

            if(arena.GetUsedBytes() > 16 * 1024 * 1024)
                arena.Clear();
        }
    }
}

PathTracingRenderer::PathTracingRenderer(int workerCount, uint32_t spp, uint32_t taskGridSize, const PathTracingIntegrator &integrator)
    : workerCount_(workerCount), spp_(spp), taskGridSize_(taskGridSize), integrator_(integrator)
{

}

void PathTracingRenderer::Render(const Scene &scene, RenderTarget *rt, ProgressReporter *reporter) const
{
    AGZ_ASSERT(rt->IsAvailable());

    std::queue<SubareaRect> tasks = GridDivider<uint32_t>::Divide(
        { 0, rt->GetWidth(), 0, rt->GetHeight() }, taskGridSize_, taskGridSize_);

    if(reporter)
        reporter->Begin();

    std::atomic<size_t> finishedCount = 0;
    size_t totalCount = tasks.size();

    auto func = [&](const SubareaRect &subarea, AGZ::NoSharedParam_t)
    {
        RenderSubarea(scene, rt, subarea);
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
