#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include <Atrc/Common.h>
#include <Atrc/Renderer/Renderer.h>
#include <Atrc/Renderer/SubareaRenderer.h>

AGZ_NS_BEG(Atrc)

template<typename SR>
class ParallelRenderer
    : ATRC_IMPLEMENTS Renderer,
      ATRC_PROPERTY AGZ::Uncopiable
{
    int workerCount_;
    SR subareaRenderer_;
    
    using Area = SubareaRenderer::Subarea;
    
    struct Param
    {
        const Scene &scene;
        const Integrator &integrator;
        RenderTarget<Color3f> &output;
        const SubareaRenderer &subareaRenderer;
    };
    
    static void Worker(
        std::mutex *_mut, std::queue<Area> *_tasks, Param param)
    {
        auto &mut = *_mut;
        auto &tasks = *_tasks;
        
        for(;;)
        {
            bool done = true;
            Area area = { 0, 0, 0, 0 };
            
            {
                std::lock_guard<std::mutex> lk(mut);
                if(!tasks.empty())
                {
                    area = tasks.front();
                    tasks.pop();
                    done = false;
                }
            }
            
            if(done)
                break;

            param.subareaRenderer.Render(
                param.scene, param.integrator, param.output, area);
        }
    }
    
public:

    template<typename...Args>
    explicit ParallelRenderer(int workerCount, Args&&...args)
        : subareaRenderer_(std::forward<Args>(args)...)
    {
        if(workerCount <= 0)
            workerCount = std::thread::hardware_concurrency();
        workerCount_ = (std::max)(1, workerCount);
    }

    void Render(
        const Scene &scene, const Integrator &integrator,
        RenderTarget<Color3f> &output) const override
    {
        AGZ_ASSERT(workerCount_ > 0);
        AGZ_ASSERT(output.IsAvailable());
        uint32_t w = output.GetWidth();
        uint32_t h = output.GetHeight();
        
        Param param = { scene, integrator, output, subareaRenderer_ };
        
        std::queue<Area> tasks;
        uint32_t yStep = w >= 1024 ? 1 : 2048 / w;
        uint32_t y = 0;
        for(; y + yStep <= h; y += yStep)
            tasks.push({ 0, w, y, y + yStep });
        if(y < h)
            tasks.push({ 0, w, y, h });
        
        std::mutex mut;
        std::vector<std::thread> workers;
        for(int i = 1; i < workerCount_; ++i)
            workers.emplace_back(Worker, &mut, &tasks, param);
        Worker(&mut, &tasks, param);
        
        for(auto &worker : workers)
            worker.join();
    }
};

AGZ_NS_END(Atrc)
