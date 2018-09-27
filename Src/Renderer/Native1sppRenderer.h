#pragma once

#include "../Common.h"
#include "../Core/Renderer.h"

AGZ_NS_BEG(Atrc)

class Native1sppRenderer
    : ATRC_IMPLEMENTS Renderer,
      ATRC_PROPERTY AGZ::Uncopiable
{
public:

    void Render(
        const Scene &scene, const Integrator &integrator,
        RenderTarget<Color3f> &target) const override;
};

AGZ_NS_END(Atrc)
