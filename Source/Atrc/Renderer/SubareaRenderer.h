#pragma once

#include <Atrc/Common.h>
#include <Atrc/Integrator/Integrator.h>

AGZ_NS_BEG(Atrc)

ATRC_INTERFACE SubareaRenderer
{
public:

    struct Subarea
    {
        uint32_t xBegin, xEnd;
        uint32_t yBegin, yEnd;
    };

    virtual ~SubareaRenderer() = default;

    virtual void Render(
        const Scene &scene, const Integrator &integrator,
        RenderTarget<Color3f> &output, const Subarea &area) const = 0;
};

AGZ_NS_END(Atrc)
