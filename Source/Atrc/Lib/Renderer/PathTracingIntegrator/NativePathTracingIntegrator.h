#pragma once

#include <Atrc/Lib/Renderer/PathTracingRenderer.h>

namespace Atrc
{

class NativePathTracingIntegrator : public PathTracingIntegrator
{
    int minDepth_, maxDepth_;
    Real contProb_;

public:

    NativePathTracingIntegrator(int minDepth, int maxDepth, Real contProb) noexcept;

    Spectrum Eval(const Scene &scene, const Ray &r, Sampler *sampler, Arena &arena) const override;
};

} // namespace Atrc
