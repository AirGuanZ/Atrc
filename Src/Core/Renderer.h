#pragma once

#include "../Common.h"
#include "../Core/Integrator.h"

AGZ_NS_BEG(Atrc)

template<typename T>
using RenderTarget = AGZ::Tex::Texture2D<T>;

ATRC_INTERFACE Renderer
{
public:

    virtual ~Renderer() = default;

    virtual void Render(
        const Scene &scene, const Integrator &integrator,
        RenderTarget<Color3f> &target) const = 0;
};

AGZ_NS_END(Atrc)
