#pragma once

#include <Atrc/Common.h>
#include <Atrc/Renderer/SerialRenderer.h>

AGZ_NS_BEG(Atrc)

class Native1sppSubareaRenderer
    : ATRC_IMPLEMENTS SubareaRenderer,
      ATRC_PROPERTY AGZ::Uncopiable
{
public:

    void Render(
        const Scene &scene, const Integrator &integrator,
        RenderTarget<Color3f> &output, const Subarea& area) const override;
};

AGZ_NS_END(Atrc)
