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

PathTracingRenderer::PathTracingRenderer(
    int workerCount, int taskGridSize, int epoch, bool shuffle, const PathTracingIntegrator &integrator) noexcept
    : taskGridSize_(taskGridSize), dispatcher_(workerCount),
      totalCount_(0), finishedCount_(0),
      totalEpoch_(epoch), shuffle_(shuffle),
      integrator_(integrator)
{
    AGZ_ASSERT(taskGridSize > 0);
}

void PathTracingRenderer::Render(const Scene *scene, Sampler *sampler, Film *film, Reporter *reporter)
{
    AGZ_ASSERT(sampler && film && reporter);

    auto resolution = film->GetResolution();

    std::vector<Grid> tasks0;
    GridDivider<int32_t>::Divide(
        Grid{ { 0, 0 }, { resolution.x, resolution.y } },
        taskGridSize_, taskGridSize_, std::back_inserter(tasks0));

    std::queue<Task> tasks;
    for(size_t i = 0; i < totalEpoch_; ++i)
    {
        if(shuffle_)
        {
            std::default_random_engine rng(
                static_cast<std::default_random_engine::result_type>(42 + i * totalEpoch_));
            std::shuffle(tasks0.begin(), tasks0.end(), rng);
        }
        for(auto &g : tasks0)
            tasks.push({ g, static_cast<int>(i) });
    }

    auto func = [=](const Task &task, AGZ::NoSharedParam_t)
    {
        int32_t taskID = task.grid.low.x * int32_t(totalCount_) * task.epoch + task.grid.low.y;
        auto gridSampler = sampler->Clone(taskID);

        auto filmGrid = film->CreateFilmGrid(task.grid);
        RenderGrid(scene, &filmGrid, gridSampler.get());

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
