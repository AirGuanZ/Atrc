#pragma once

#include <Atrc/Core/Core.h>

AGZ_NS_BEG(Atrc)

class JitteredSubareaRenderer : public SubareaRenderer
{
    uint32_t spp_;

public:

    explicit JitteredSubareaRenderer(uint32_t spp);

    void Render(const Scene &scene, const Integrator &integrator, RenderTarget &rt, const SubareaRect &area) const override;
};

AGZ_NS_END(Atrc)
