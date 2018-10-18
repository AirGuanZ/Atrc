#pragma once

#include <Atrc/Core/Common.h>
#include <Atrc/Core/Integrator.h>
#include <Atrc/Core/Scene.h>

AGZ_NS_BEG(Atrc)

class Renderer
{
public:

    virtual ~Renderer() = default;

    virtual void Render(const Integrator &integrator, const Scene &scene, RenderTarget &rt) const = 0;
};

AGZ_NS_END(Atrc)
