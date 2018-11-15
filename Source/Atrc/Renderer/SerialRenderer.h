#pragma once

#include <iostream>
#include <Atrc/Core/Core.h>

AGZ_NS_BEG(Atrc)

class SerialRenderer : public Renderer
{
public:

    void Render(
        const SubareaRenderer &subareaRenderer, const Scene &scene, const Integrator &integrator,
        RenderTarget *rt, ProgressReporter *reporter) const override
    {
        AGZ_ASSERT(rt.IsAvailable());

        uint32_t w = rt->GetWidth();
        uint32_t h = rt->GetHeight();
        uint32_t yStep = w >= 256 ? 1 : 512 / w;
        uint32_t y = 0;
        try
        {
            for(; y + yStep <= h; y += yStep)
            {
                subareaRenderer.Render(scene, integrator, *rt, { 0, w, y, y + yStep });
                if(reporter)
                    reporter->Report(100.0 * (y + yStep) / h);
            }

            if(y < h)
            {
                subareaRenderer.Render(scene, integrator, *rt, { 0, w, y, h });
                if(reporter)
                    reporter->Report(100.0);
            }
        }
        catch(const std::exception &err)
        {
            if(reporter)
                reporter->Message(Str8("Exception in rendering thread: ") + err.what());
        }
        catch(...)
        {
            if(reporter)
                reporter->Message("Some error occurred...");
        }
        
        std::cout << std::endl;
    }
};

AGZ_NS_END(Atrc)
