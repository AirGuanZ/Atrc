#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

#include "../Common.h"
#include "../Core/Renderer.h"
#include "SubareaRenderer.h"

AGZ_NS_BEG(Atrc)

template<typename SubareaRenderer>
class NativeParallelRendererImpl
    : ATRC_PROPERTY AGZ::Uncopiable
{
    struct RenderParameters
    {
        RenderTarget<Color3f> *renderTarget = nullptr;
        const Scene *scene                  = nullptr;
        const Integrator *integrator        = nullptr;
    };

    std::condition_variable cond_var_;

    std::mutex mut_;
    RenderParameters params_;
    std::queue<RenderTargetSubarea> taskQueue_;

    std::atomic<bool> continue_ = false;

    void Worker()
    {
        while(continue_)
        {
            RenderTargetSubarea area;
            bool nonEmpty = false;

            {
                std::lock_guard<std::mutex> lk(mut_);
                if(!taskQueue_.empty())
                {
                    area = taskQueue_.front();
                    taskQueue_.pop();
                    nonEmpty = true;
                }
            }

            if(nonEmpty)
            {
                cond_var_.notify_one();
                SubareaRenderer().Render(
                    *params_.scene, *params_.integrator, *params_.renderTarget, area);
            }
        }
    }

public:

    void Render(
        const Scene &scene, const Integrator &integrator,
        RenderTarget<Color3f> &target, uint32_t workerCount)
    {
        continue_ = true;

        if(!workerCount)
        {
            SubareaRenderer().Render(
                scene, integrator, target,
                { 0, target.GetWidth(), 0, target.GetHeight() });
            return;
        }

        std::vector<std::thread> workers;
        while(workerCount-- > 0)
            workers.emplace_back([this]() { this->Worker(); });

        params_.renderTarget = &target;
        params_.scene        = &scene;
        params_.integrator   = &integrator;

        // How many lines each task contains
        uint32_t yStep = target.GetWidth() > 512 ? 1 : 512 / target.GetWidth();
        uint32_t yEnd = target.GetHeight() / yStep * yStep;

        {
            std::lock_guard<std::mutex> lk(mut_);

            uint32_t y = 0;
            for(; y + yStep <= yEnd; y += yStep)
                taskQueue_.push({ 0, target.GetWidth(), y, y + yStep });

            if(yEnd != target.GetHeight())
                taskQueue_.push({ 0, target.GetWidth(), y, target.GetHeight() });
        }

        std::unique_lock<std::mutex> lk(mut_);
        cond_var_.wait(lk, [this]() { return taskQueue_.empty(); });
        
        AGZ_ASSERT(taskQueue_.empty());

        continue_ = false;
        for(auto &worker : workers)
            worker.join();
    }
};

template<typename SubareaRenderer>
class NativeParallelRenderer
    : ATRC_IMPLEMENTS Renderer,
      ATRC_PROPERTY AGZ::Uncopiable
{
    uint32_t workerCount_;

public:

    explicit NativeParallelRenderer(int workerCount = -1)
    {
        workerCount_ = workerCount < 0 ? std::thread::hardware_concurrency()
                                       : workerCount;
    }

    void Render(
        const Scene &scene, const Integrator &integrator,
        RenderTarget<Color3f> &target) const override
    {
        NativeParallelRendererImpl<SubareaRenderer> impl;
        impl.Render(scene, integrator, target, workerCount_);
    }
};

AGZ_NS_END(Atrc)
