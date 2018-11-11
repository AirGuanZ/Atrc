#pragma once

#include <iostream>
#include <Atrc/Core/Core.h>

AGZ_NS_BEG(Atrc)

class SerialRenderer : public Renderer
{
public:

    void Render(const SubareaRenderer &subareaRenderer, const Scene &scene, const Integrator &integrator, RenderTarget &rt) const override
    {
        AGZ_ASSERT(rt.IsAvailable());

        uint32_t w = rt.GetWidth();
        uint32_t h = rt.GetHeight();
        uint32_t yStep = w >= 256 ? 1 : 512 / w;
        uint32_t y = 0;
        try
        {
            for(; y + yStep <= h; y += yStep)
            {
                subareaRenderer.Render(scene, integrator, rt, { 0, w, y, y + yStep });

                float percent = 100.0f * (y + yStep) / h;
                std::printf("%sProgress: %5.2f%%  ",
                             std::string(50, '\b').c_str(), percent);
            }

            if(y < h)
            {
                subareaRenderer.Render(scene, integrator, rt, { 0, w, y, h });
                std::printf("%sProgress: %5.2f%%  ",
                            std::string(50, '\b').c_str(), 100.0f);
            }
        }
        catch(const std::exception &err)
        {
            std::printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b"
                        "Exception in rendering thread: %s\n", err.what());
            printf("Some error occurred...\n");
            std::terminate();
        }
        catch(...)
        {
            std::printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b"
                        "Unknown exception in rendering thread\n");
            printf("Some error occurred...\n");
            std::terminate();
        }
        
        std::cout << std::endl;
    }
};

AGZ_NS_END(Atrc)
