#include <atomic>
#include <mutex>

#include <AGZUtils/Utils/Thread.h>

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

                Spectrum value = w * integrator_.Eval(scene, r, sampler, arena) / pdf;
                AGZ_ASSERT(!value.HasInf());
                filmGrid->AddSample(camSam.film, value);

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

void PathTracingRenderer::Render(const Scene &scene, Sampler *sampler, Film *film, Reporter *reporter) const
{
    AGZ_ASSERT(sampler && film && reporter);

    auto resolution = film->GetResolution();
    std::queue<Grid> tasks = GridDivider<int32_t>::Divide(
        { { 0, 0 }, { resolution.x, resolution.y } }, taskGridSize_, taskGridSize_);

    size_t totalTaskCount = tasks.size();
    std::atomic<size_t> finishedTaskCount = 0;

    std::mutex mergeMut;

    auto func = [&](const Grid &task, AGZ::NoSharedParam_t)
    {
        int32_t taskID = task.low.x * int32_t(tasks.size()) + task.low.y;
        auto gridSampler = sampler->Clone(taskID);

        auto filmGrid = film->CreateFilmGrid(task);
        RenderGrid(scene, &filmGrid, gridSampler.get());
        
        film->MergeFilmGrid(filmGrid);

        // filmGrid间无任何overlap，故只需要对reporter加锁即可
        std::lock_guard<std::mutex> lk(mergeMut);

        Real percent = Real(100) * ++finishedTaskCount / totalTaskCount;
        reporter->Report(*film, percent);
    };

    AGZ::StaticTaskDispatcher<Grid, AGZ::NoSharedParam_t> dispatcher(workerCount_);

    reporter->Start();
    bool ok = dispatcher.Run(func, AGZ::NO_SHARED_PARAM, tasks);

    if(!ok)
    {
        reporter->Message("Something was wrong...");
        for(auto &err : dispatcher.GetExceptions())
            reporter->Message(err.what());
    }
    else
        reporter->End();
}

} // namespace Atrc
