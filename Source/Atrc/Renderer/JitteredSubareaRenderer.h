#pragma once

#include <Atrc/Common.h>
#include <Atrc/Renderer/SubareaRenderer.h>

AGZ_NS_BEG(Atrc)

class JitteredSubareaRenderer
    : ATRC_IMPLEMENTS SubareaRenderer,
      ATRC_PROPERTY AGZ::Uncopiable
{
    uint32_t spp_;

public:

    explicit JitteredSubareaRenderer(uint32_t spp);

    void Render(
        const Scene &scene, const Integrator &integrator,
        RenderTarget<Color3f> &output, const Subarea &area) const override;
};

AGZ_NS_END(Atrc)
