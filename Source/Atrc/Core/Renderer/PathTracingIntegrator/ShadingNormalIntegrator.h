#pragma once

#include <Atrc/Core/Renderer/PathTracingRenderer.h>

namespace Atrc
{

class ShadingNormalIntegrator : public PathTracingIntegrator
{
public:

    Spectrum Eval(const Scene &scene, const Ray &r, Sampler *sampler, Arena &arena) const override;
};

} // namespace Atrc
