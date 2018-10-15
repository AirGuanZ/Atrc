#pragma once

#include <iostream>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

#include <Atrc/Common.h>
#include <Atrc/Renderer/Renderer.h>
#include <Atrc/Renderer/SubareaRenderer.h>

AGZ_NS_BEG(Atrc)

template<typename SR, std::enable_if_t<std::is_base_of_v<SubareaRenderer, SR>, int> = 0>
class ParallelRenderer
    : ATRC_IMPLEMENTS Renderer,
      ATRC_PROPERTY AGZ::Uncopiable
{
    int workerCount_;
    SR subareaRenderer_;

    bool printProgress_;
    
    using Area = SubareaRenderer::Subarea;
    
    struct Param
    {
        const Scene           *scene;
        const Integrator      *integrator;
        RenderTarget<Color3f> *output;
        const SubareaRenderer *subareaRenderer;

        std::mutex       *mut;
        std::queue<Area> *tasks;

        std::mutex *outMut = nullptr;
        size_t taskCount   = 0;
    };
    
    static void Worker(Param param)
    {
        for(;;)
        {
            bool done = true;
            Area area = { 0, 0, 0, 0 };
            
            {
                std::lock_guard<std::mutex> lk(*param.mut);
                if(!param.tasks->empty())
                {
                    if(param.outMut)
                    {
                        std::lock_guard<std::mutex> lk2(*param.outMut);
                        float percent = 100.0f * param.tasks->size() / param.taskCount;
                        printf("%sRemaining tasks: %5.2f%%  ",
                               std::string(50, '\b').c_str(), percent);
                    }

                    area = param.tasks->front();
                    param.tasks->pop();
                    done = false;
                }
            }
            
            if(done)
                break;

            param.subareaRenderer->Render(
                *param.scene, *param.integrator, *param.output, area);
        }
    }
    
public:

    template<typename...Args>
    explicit ParallelRenderer(int workerCount, Args&&...args)
        : subareaRenderer_(std::forward<Args>(args)...), printProgress_(false)
    {
        if(workerCount <= 0)
            workerCount = std::thread::hardware_concurrency();
        workerCount_ = (std::max)(1, workerCount) - 1;
    }

    void SetProgressPrinting(bool print)
    {
        printProgress_ = print;
    }

    void Render(
        const Scene &scene, const Integrator &integrator,
        RenderTarget<Color3f> &output) const override
    {
        AGZ_ASSERT(output.IsAvailable());
        uint32_t w = output.GetWidth();
        uint32_t h = output.GetHeight();
        
        std::queue<Area> tasks;
        uint32_t yStep = w >= 4096 ? 1 : 8192 / w;
        uint32_t y = 0;
        for(; y + yStep <= h; y += yStep)
            tasks.push({ 0, w, y, y + yStep });
        if(y < h)
            tasks.push({ 0, w, y, h });

        std::mutex mut, outMut;
        Param param = {
            &scene, &integrator, &output, &subareaRenderer_,
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
            workers.emplace_back(Worker, param);
        Worker(param);
        
        for(auto &worker : workers)
            worker.join();

        if(printProgress_)
            std::cout << std::endl;
    }
};

AGZ_NS_END(Atrc)
