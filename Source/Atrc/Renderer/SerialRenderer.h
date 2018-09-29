#pragma once

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

public:

    template<typename...Args>
    explicit SerialRenderer(Args&&...args)
        : subareaRenderer_(std::forward<Args>(args)...)
    {
        
    }

    void Render(
        const Scene &scene, const Integrator &integrator,
        RenderTarget<Color3f> &output) const override
    {
        AGZ_ASSERT(output.IsAvailable());
        subareaRenderer_.Render(
            scene, integrator, output,
            { 0, output.GetWidth(), 0, output.GetHeight() });
    }
};

AGZ_NS_END(Atrc)
