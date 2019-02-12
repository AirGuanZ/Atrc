#include <atomic>
#include <mutex>

#include <AGZUtils/Utils/Thread.h>

#include <Atrc/Lib/Core/Reporter.h>
#include <Atrc/Lib/Core/Sampler.h>
#include <Atrc/Lib/Core/Scene.h>
#include <Atrc/Lib/Core/TFilm.h>
#include <Atrc/Lib/Renderer/PathTracingRenderer.h>

namespace Atrc
{

void PathTracingRenderer::RenderGrid(const Scene *scene, FilmGrid *filmGrid, Sampler *sampler) const
{
    Arena arena;
    
    auto cam = scene->GetCamera();
    auto rect = filmGrid->GetSamplingRect();

    for(int32_t py = rect.low.y; py < rect.high.y; ++py)
    {
        for(int32_t px = rect.low.x; px < rect.high.x; ++px)
        {
            sampler->StartPixel({ px, py });
            do {
                auto camSam = sampler->GetCameraSample();
                auto [r, w, pdf] = cam->GenerateRay(camSam);

                Spectrum value = w * integrator_.Eval(*scene, r, sampler, arena) / pdf;
                AGZ_ASSERT(!value.HasInf());
                filmGrid->AddSample(camSam.film, value);

                if(arena.GetUsedBytes() > 16 * 1024 * 1024)
                    arena.Clear();

            } while(sampler->NextSample());
        }
    }
}

PathTracingRenderer::PathTracingRenderer(int workerCount, int taskGridSize, const PathTracingIntegrator &integrator) noexcept
    : taskGridSize_(taskGridSize), dispatcher_(workerCount), totalCount_(0), finishedCount_(0), integrator_(integrator)
{
    AGZ_ASSERT(taskGridSize > 0);
}

void PathTracingRenderer::Render(const Scene *scene, Sampler *sampler, Film *film, Reporter *reporter)
{
    AGZ_ASSERT(sampler && film && reporter);

    auto resolution = film->GetResolution();
    std::queue<Grid> tasks = GridDivider<int32_t>::Divide(
        { { 0, 0 }, { resolution.x, resolution.y } }, taskGridSize_, taskGridSize_);

    auto func = [=](const Grid &task, AGZ::NoSharedParam_t)
    {
        int32_t taskID = task.low.x * int32_t(totalCount_) + task.low.y;
        auto gridSampler = sampler->Clone(taskID);

        auto filmGrid = film->CreateFilmGrid(task);
        RenderGrid(scene, &filmGrid, gridSampler.get());

        // filmGrid间无任何overlap，故只需要对reporter加锁即可
        std::lock_guard<std::mutex> lk(mergeMut_);
        
        film->MergeFilmGrid(filmGrid);

        Real percent = Real(100) * ++finishedCount_ / totalCount_;
        reporter->Report(*film, percent);
    };

    reporter->Start();

    totalCount_ = tasks.size();
    finishedCount_ = 0;
    dispatcher_.RunAsync(std::move(func), AGZ::NO_SHARED_PARAM, std::move(tasks));
}

bool PathTracingRenderer::IsCompleted() const
{
    return dispatcher_.IsCompleted();
}

void PathTracingRenderer::Join(Reporter *reporter)
{
    if(!dispatcher_.Join())
    {
        reporter->Message("Something was wrong...");
        for(auto &err : dispatcher_.GetExceptions())
            reporter->Message(err.what());
    }
    else
        reporter->End();
}

void PathTracingRenderer::Stop()
{
	dispatcher_.Stop();
	totalCount_ = 0;
	finishedCount_ = 0;
}

} // namespace Atrc
