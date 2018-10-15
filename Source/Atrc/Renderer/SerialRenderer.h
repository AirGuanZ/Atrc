#pragma once

#include <iostream>

#include <Atrc/Common.h>
#include <Atrc/Renderer/Renderer.h>
#include <Atrc/Renderer/SubareaRenderer.h>

AGZ_NS_BEG(Atrc)

template<typename SR,
         std::enable_if_t<std::is_base_of_v<SubareaRenderer, SR>,
                          int> = 0>
class SerialRenderer
    : ATRC_IMPLEMENTS Renderer,
      ATRC_PROPERTY AGZ::Uncopiable
{
    SR subareaRenderer_;
    bool printProgress_;

public:

    template<typename...Args>
    explicit SerialRenderer(Args&&...args)
        : subareaRenderer_(std::forward<Args>(args)...), printProgress_(false)
    {
        
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
        uint32_t yStep = w >= 4096 ? 1 : 8192 / w;
        uint32_t y = 0;

        for(; y + yStep <= h; y += yStep)
        {
            subareaRenderer_.Render(
                scene, integrator, output,
                { 0, w, y, y + yStep });
            if(printProgress_)
            {
                float percent = 100.0f * (h - y) / h;
                    std::printf("%sRemaining tasks: %5.2f%%  ",
                                std::string(50, '\b').c_str(), percent);
            }
        }
        if(y < h)
        {
            subareaRenderer_.Render(
                scene, integrator, output,
                { 0, w, y, h });
        }

        if(printProgress_)
            std::cout << std::endl;
    }
};

AGZ_NS_END(Atrc)
