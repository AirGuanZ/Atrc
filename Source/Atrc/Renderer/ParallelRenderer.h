#pragma once

#include <iostream>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

#include <Atrc/Core/Core.h>

AGZ_NS_BEG(Atrc)

class ParallelRenderer : public Renderer
{
    int workerCount_;

    bool printProgress_;

    struct Param
    {
        const Scene           *scene           = nullptr;
        const Integrator      *integrator      = nullptr;
        RenderTarget          *output          = nullptr;
        const SubareaRenderer *subareaRenderer = nullptr;

        std::mutex              *mut   = nullptr;
        std::queue<SubareaRect> *tasks = nullptr;

        std::mutex *outMut   = nullptr;
        size_t taskCount     = 0;
        size_t finishedCount = 0;
    };

    static void Worker(Param &param)
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
                if(param.outMut)
                {
                    std::lock_guard<std::mutex> lk2(*param.outMut);
                    float percent = 100.0f * (++param.finishedCount) / param.taskCount;
					std::cout << "Progress: " << percent << "%" << std::endl;
                }
            }
        }
        catch(const std::exception &err)
        {
            if(param.outMut)
            {
                std::lock_guard<std::mutex> lk(*param.outMut);
				std::cout << "Exception in subrendering thread: " << err.what() << std::endl;
            }
            std::terminate();
        }
        catch(...)
        {
            if(param.outMut)
            {
                std::lock_guard<std::mutex> lk(*param.outMut);
				std::cout << "Unknown exception in subrendering thread" << std::endl;
            }
            std::terminate();
        }
    }

public:

    explicit ParallelRenderer(int workerCount = -1)
        : printProgress_(false)
    {
        if(workerCount <= 0)
            workerCount = std::thread::hardware_concurrency();
        workerCount_ = (std::max)(1, workerCount) - 1;
    }

    void EnableProgressPrinting(bool print)
    {
        printProgress_ = print;
    }

    void Render(const SubareaRenderer &subareaRenderer, const Scene &scene, const Integrator &integrator, RenderTarget &output) const override
    {
        AGZ_ASSERT(output.IsAvailable());
        uint32_t w = output.GetWidth();
        uint32_t h = output.GetHeight();

        std::queue<SubareaRect> tasks;
        uint32_t yStep = w >= 1024 ? 1 : 2048 / w;
        uint32_t y = 0;
        for(; y + yStep <= h; y += yStep)
            tasks.push({ 0, w, y, y + yStep });
        if(y < h)
            tasks.push({ 0, w, y, h });

        std::mutex mut, outMut;
        Param param = {
            &scene, &integrator, &output, &subareaRenderer,
            &mut, &tasks
        };

        if(printProgress_)
        {
            param.outMut = &outMut;
            param.taskCount = tasks.size();
        }

        std::vector<std::thread> workers;
        if(workerCount_)
            workers.reserve(workerCount_);
        for(int i = 0; i < workerCount_; ++i)
            workers.emplace_back(Worker, std::ref(param));
        Worker(param);

        for(auto &worker : workers)
            worker.join();

        if(!tasks.empty())
            std::cout << "Some error occurred..." << std::endl;

        if(printProgress_)
            std::cout << std::endl;
    }
};

AGZ_NS_END(Atrc)
