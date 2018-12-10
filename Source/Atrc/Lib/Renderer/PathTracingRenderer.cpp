#include <vector>
#include <Utils/Thread.h>

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
                auto [r, qe, pdf] = cam->GenerateRay({ px + xOffset, py + yOffset });
                pixel += (qe / float(pdf)) * integrator_.Eval(scene, r, arena);
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

void PathTracingRenderer::Render(const Scene &scene, RenderTarget *rt) const
{
    AGZ_ASSERT(rt->IsAvailable());

    std::queue<SubareaRect> tasks = GridDivider<uint32_t>::Divide(
        { 0, rt->GetWidth(), 0, rt->GetHeight() }, taskGridSize_, taskGridSize_);

    auto func = [&](const SubareaRect &subarea, AGZ::NoSharedParam_t)
    {
        RenderSubarea(scene, rt, subarea);
    };

    AGZ::StaticTaskDispatcher<SubareaRect, AGZ::NoSharedParam_t> dispatcher(workerCount_);

    dispatcher.Run(func, AGZ::NO_SHARED_PARAM, tasks);
}

AGZ_NS_END(Atrc)
