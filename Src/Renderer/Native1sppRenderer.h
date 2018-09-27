#pragma once

#include "../Common.h"
#include "../Core/Renderer.h"
#include "SubareaRenderer.h"

AGZ_NS_BEG(Atrc)

class Native1sppSubareaRenderer
    : ATRC_PROPERTY AGZ::Uncopiable
{
public:

    void Render(
        const Scene &scene, const Integrator &integrator,
        RenderTarget<Color3f> &target,
        const RenderTargetSubarea &area) const;
};

using Native1sppRenderer = NativeSubareaRendererWrapper<Native1sppSubareaRenderer>;

AGZ_NS_END(Atrc)
