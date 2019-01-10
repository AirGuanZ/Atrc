#pragma once

#include <AGZUtils/Utils/Texture.h>

#include <Atrc/Lib/Core/Reporter.h>
#include <Atrc/Lib/Core/Sampler.h>
#include <Atrc/Lib/Core/Scene.h>
#include <Atrc/Lib/Core/TFilm.h>

namespace Atrc
{

class Renderer
{
public:

    virtual ~Renderer() = default;

    virtual void Render(const Scene &scene, Sampler *sampler, Film *film, Reporter *reporter) const = 0;
};

} // namespace Atrc
