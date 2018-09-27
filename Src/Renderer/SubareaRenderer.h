#pragma once

#include "../Common.h"

AGZ_NS_BEG(Atrc)

// Rectangle on render target
// Coord System Origin: top-left pixel
struct RenderTargetSubarea
{
    uint32_t xBegin = 0, xEnd = 0,
             yBegin = 0, yEnd = 0;
};

/*
    Concept: SubareaRenderer
    {
        void Render(
            const Scene &scene, const Integrator &integrator,
            RenderTarget<Color3f> &target,
            const RenderTargetSubarea &area) const;
    }
*/

template<typename SubareaRenderer>
class NativeSubareaRendererWrapper
    : ATRC_IMPLEMENTS Renderer,
      ATRC_PROPERTY AGZ::Uncopiable
{
public:

    void Render(
        const Scene &scene, const Integrator &integrator,
        RenderTarget<Color3f> &target) const override
    {
        AGZ_ASSERT(target.IsAvailable());
        SubareaRenderer().Render(
            scene, integrator, target,
            { 0, target.GetWidth(), 0, target.GetHeight() });
    }
};

AGZ_NS_END(Atrc)
