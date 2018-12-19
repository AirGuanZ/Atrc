#include <Atrc/Lib/Renderer/PathTracingIntegrator/VolumetricPathTracingIntegrator.h>

namespace Atrc
{

VolumetricPathTracingIntegrator::VolumetricPathTracingIntegrator(
    int minDepth, int maxDepth, Real contProb, bool sampleAllLights) noexcept
    : minDepth_(minDepth), maxDepth_(maxDepth), contProb_(contProb), sampleAllLights_(sampleAllLights)
{
    AGZ_ASSERT(0 < minDepth && minDepth <= maxDepth);
    AGZ_ASSERT(0 < contProb && contProb <= 1);
}

Spectrum VolumetricPathTracingIntegrator::Eval(const Scene &scene, const Ray &_r, Sampler *sampler, Arena &arena) const
{
    Spectrum coef = Spectrum(Real(1)), ret;
    Ray r = _r;

    for(int depth = 1; depth <= maxDepth_; ++depth)
    {
        // Russian roulette strategy

        if(depth > minDepth_)
        {
            if(sampler->GetReal() > contProb_)
                break;
            coef /= contProb_;
        }

        // TODO
    }

    return ret;
}

} // namespace Atrc
