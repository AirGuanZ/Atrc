#pragma once

#include <Utils/Texture.h>

#include <Atrc/Lib/Core/Film.h>
#include <Atrc/Lib/Core/Reporter.h>
#include <Atrc/Lib/Core/Sampler.h>
#include <Atrc/Lib/Core/Scene.h>

namespace Atrc
{

class Renderer
{
public:

    virtual ~Renderer() = default;

    virtual void Render(const Scene &scene, Sampler *sampler, Film *film, Reporter *reporter) const = 0;
};

} // namespace Atrc
