#pragma once

#include <Atrc/Lib/Renderer/PathTracingRenderer.h>

namespace Atrc
{

class VolPathTracingIntegrator : public PathTracingIntegrator
{
    int minDepth_, maxDepth_;
    Real contProb_;

    bool sampleAllLights_;
    bool lightInMedium_;

public:

    VolPathTracingIntegrator(int minDepth, int maxDepth, Real contProb, bool sampleAllLights, bool lightInMedium) noexcept;

    Spectrum Eval(const Scene &scene, const Ray &_r, Sampler *sampler, Arena &arena) const override;
};

} // namespace Atrc
