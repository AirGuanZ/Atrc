#include <vector>

#include "ParallelRenderer.h"

AGZ_NS_BEG(Atrc)

void ParallelRenderer::Worker(Param &param)
{
    try
    {
        for(;;)
        {
            bool done = true;
            SubareaRect area = { 0, 0, 0, 0 };

            {
                std::lock_guard<std::mutex> lk(*param.mut);
                if(!param.tasks->empty())
                {
                    area = param.tasks->front();
                    param.tasks->pop();
                    done = false;
                }
            }

            if(done)
                break;

            param.subareaRenderer->Render(
                *param.scene, *param.integrator, *param.output, area);
            if(param.reporter)
                param.reporter->Report(100.0 * (++param.finishedCount) / param.taskCount);
        }
    }
    catch(const std::exception &err)
    {
        if(param.reporter)
            param.reporter->Message(Str8("Exception in subrendering thread: ") + err.what());
    }
    catch(...)
    {
        if(param.reporter)
            param.reporter->Message("Unknown exception in subrendering thread");
    }
}

ParallelRenderer::ParallelRenderer(int workerCount)
{
    if(workerCount <= 0)
        workerCount = std::thread::hardware_concurrency();
    workerCount_ = (std::max)(1, workerCount) - 1;
}

void ParallelRenderer::Render(
    const SubareaRenderer &subareaRenderer, const Scene &scene, const Integrator &integrator,
    RenderTarget *output, ProgressReporter *reporter) const
{
    AGZ_ASSERT(output.IsAvailable());
    uint32_t w = output->GetWidth();
    uint32_t h = output->GetHeight();

    std::queue<SubareaRect> tasks;
    uint32_t yStep = w >= 256 ? 1 : 512 / w;
    uint32_t y = 0;
    for(; y + yStep <= h; y += yStep)
        tasks.push({ 0, w, y, y + yStep });
    if(y < h)
        tasks.push({ 0, w, y, h });

    std::mutex mut, reportMut;
    Param param = {
        &scene, &integrator, output, &subareaRenderer,
        &mut, &tasks
    };

    param.reporter  = reporter;
    param.reportMut = &reportMut;
    param.taskCount = tasks.size();

    std::vector<std::thread> workers;
    if(workerCount_)
        workers.reserve(workerCount_);
    for(int i = 0; i < workerCount_; ++i)
        workers.emplace_back(Worker, std::ref(param));
    Worker(param);

    for(auto &worker : workers)
        worker.join();
}

AGZ_NS_END(Atrc)
