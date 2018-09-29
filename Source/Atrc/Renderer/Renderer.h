#pragma once

#include <Atrc/Common.h>
#include <Atrc/Integrator/Integrator.h>

AGZ_NS_BEG(Atrc)

ATRC_INTERFACE Renderer
{
public:

    virtual ~Renderer() = default;

    virtual void Render(
        const Scene &scene, const Integrator &integrator,
        RenderTarget<Color3f> &output) = 0;
};

AGZ_NS_END(Atrc)
