#pragma once

#include <Atrc/Lib/Renderer/PathTracingRenderer.h>

namespace Atrc
{

class MISPathTracingIntegrator : public PathTracingIntegrator
{
    int minDepth_, maxDepth_;
    Real contProb_;

    bool sampleAllLights_;

public:

    MISPathTracingIntegrator(int minDepth, int maxDepth, Real contProb, bool sampleAllLights) noexcept;

    Spectrum Eval(const Scene &scene, const Ray &r, Sampler *sampler, Arena &arena) const override;
};

} // namespace Atrc
