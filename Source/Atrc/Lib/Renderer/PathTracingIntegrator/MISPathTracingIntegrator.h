#include <Atrc/Lib/Renderer/PathTracingRenderer.h>

namespace Atrc
{

class MISPathTracingIntegrator : public PathTracingIntegrator
{
    int minDepth_, maxDepth_;
    Real contProb_;

    bool sampleAllLights_;

    Spectrum MISSampleLight(
        const Scene &scene, const Light *light,
        const Intersection &inct, const ShadingPoint &shd, const Vec3 &sample) const;

public:

    MISPathTracingIntegrator(int minDepth, int maxDepth, Real contProb, bool sampleAllLights) noexcept;

    Spectrum Eval(const Scene &scene, const Ray &r, Sampler *sampler, Arena &arena) const override;
};

} // namespace Atrc
