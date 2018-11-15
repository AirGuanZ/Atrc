#pragma once

#include <mutex>
#include <queue>
#include <thread>

#include <Atrc/Core/Core.h>

AGZ_NS_BEG(Atrc)

class ParallelRenderer : public Renderer
{
    int workerCount_;

    struct Param
    {
        const Scene           *scene           = nullptr;
        const Integrator      *integrator      = nullptr;
        RenderTarget          *output          = nullptr;
        const SubareaRenderer *subareaRenderer = nullptr;

        std::mutex              *mut   = nullptr;
        std::queue<SubareaRect> *tasks = nullptr;

        ProgressReporter *reporter = nullptr;
        std::mutex *reportMut      = nullptr;
        std::atomic<size_t> taskCount = 0;
        size_t finishedCount          = 0;
    };

    static void Worker(Param &param);

public:

    explicit ParallelRenderer(int workerCount = -1);

    void Render(
        const SubareaRenderer &subareaRenderer, const Scene &scene, const Integrator &integrator,
        RenderTarget *output, ProgressReporter *reporter) const override;
};

AGZ_NS_END(Atrc)
