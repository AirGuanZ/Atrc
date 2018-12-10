#pragma once

#include <Utils/Texture.h>

#include <Atrc/Lib/Core/Scene.h>

namespace Atrc
{

using RenderTarget = AGZ::Texture2D<Spectrum>;

class Renderer
{
public:

    virtual ~Renderer() = default;

    virtual void Render(const Scene &scene, RenderTarget *rt) const = 0;
};

} // namespace Atrc
