#include <Utils/Thread.h>

#include <Atrc/Lib/Renderer/PathTracingRenderer.h>

namespace Atrc
{

void PathTracingRenderer::RenderGrid(const Scene &scene, FilmGrid *filmGrid, Sampler *sampler) const
{
    Arena arena;
    
    auto cam = scene.GetCamera();
    auto rect = filmGrid->GetSamplingRect();

    for(int32_t py = rect.low.y; py < rect.high.y; ++py)
    {
        for(int32_t px = rect.low.x; px < rect.high.x; ++px)
        {
            sampler->StartPixel({ px, py });
            do {
                auto camSam = sampler->GetCameraSample();
                auto [r, w, pdf] = cam->GenerateRay(camSam);

                Spectrum value = integrator_.Eval(scene, r, sampler, arena); 
                filmGrid->AddSample(camSam.film, (w / pdf) * value);

                if(arena.GetUsedBytes() > 16 * 1024 * 1024)
                    arena.Clear();

            } while(sampler->NextSample());
        }
    }
}

PathTracingRenderer::PathTracingRenderer(int workerCount, int taskGridSize, const PathTracingIntegrator &integrator) noexcept
    : workerCount_(workerCount), taskGridSize_(taskGridSize), integrator_(integrator)
{
    AGZ_ASSERT(taskGridSize > 0);
}

void PathTracingRenderer::Render(const Scene &scene, Sampler *sampler, Film *film) const
{
    auto resolution = film->GetResolution();
    std::queue<Grid> tasks = GridDivider<int32_t>::Divide(
        { { 0, 0 }, { resolution.x, resolution.y } }, taskGridSize_, taskGridSize_);

    auto func = [&](const Grid &task, AGZ::NoSharedParam_t)
    {
        auto filmGrid = film->CreateFilmGrid(task);
        int32_t taskID = task.low.x * tasks.size() + task.low.y;
        auto gridSampler = sampler->Clone(taskID);
        RenderGrid(scene, &filmGrid, gridSampler.get());
        film->MergeFilmGrid(filmGrid);
    };

    AGZ::StaticTaskDispatcher<Grid, AGZ::NoSharedParam_t> dispatcher(workerCount_);
    dispatcher.Run(func, AGZ::NO_SHARED_PARAM, tasks);
}

} // namespace Atrc
