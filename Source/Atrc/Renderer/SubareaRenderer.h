#pragma once

#include <Atrc/Core/Core.h>

AGZ_NS_BEG(Atrc)

struct SubareaRect
{
    uint32_t xBegin, xEnd;
    uint32_t yBegin, yEnd;
};

class SubareaRenderer
{
public:

    virtual ~SubareaRenderer() = default;

    virtual void Render(const Scene &scene, const Integrator &integrator, RenderTarget &rt, const SubareaRect &area) const = 0;
};

AGZ_NS_END(Atrc)
