#pragma once

#include <Atrc/Core/Renderer/PathTracingRenderer.h>

namespace Atrc
{
    
class FullPathTracingIntegrator : public PathTracingIntegrator
{
    int minDepth_, maxDepth_;
    Real contProb_;

    bool sampleAllLights_;

public:

    FullPathTracingIntegrator(int minDepth, int maxDepth, Real contProb, bool sampleAllLights) noexcept;

    Spectrum Eval(const Scene &scene, const Ray &_r, Sampler *sampler, Arena &arena) const override;
};

} // namespace Atrc
