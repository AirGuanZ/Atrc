#include <Atrc/Lib/Renderer/PathTracingIntegrator/DirectLighting.h>
#include <Atrc/Lib/Renderer/PathTracingIntegrator/MISPathTracingIntegrator.h>

namespace Atrc
{

MISPathTracingIntegrator::MISPathTracingIntegrator(
    int minDepth, int maxDepth, Real contProb, bool sampleAllLights) noexcept
    : minDepth_(minDepth), maxDepth_(maxDepth), contProb_(contProb), sampleAllLights_(sampleAllLights)
{
    AGZ_ASSERT(0 < minDepth && minDepth <= maxDepth);
    AGZ_ASSERT(0 < contProb && contProb <= 1);
}

Spectrum MISPathTracingIntegrator::Eval(
    const Scene &scene, const Ray &r, Sampler *sampler, Arena &arena) const
{
    Spectrum coef(Real(1)), ret;
    Ray ray = r;

    for(int depth = 1; depth <= maxDepth_; ++depth)
    {
        if(depth > minDepth_)
        {
            if(sampler->GetReal() > contProb_)
                break;
            coef /= contProb_;
        }

        Intersection inct;
        if(!scene.FindIntersection(ray, &inct))
        {
            if(depth == 1)
            {
                for(auto light : scene.GetLights())
                    ret += coef * light->NonAreaLe(ray);
            }
            break;
        }

        if(depth == 1)
        {
            if(auto light = inct.entity->AsLight())
                ret += coef * light->AreaLe(inct);
        }

        auto shd = inct.material->GetShadingPoint(inct, arena);

        auto [directLighting, bsdfSample, nInct] = ComputeDirectLighting(scene, inct, shd, sampleAllLights_, false, sampler);

        ret += coef * directLighting;

        if(!bsdfSample)
            break;
        coef *= bsdfSample->coef * Abs(Cos(bsdfSample->wi, shd.coordSys.ez)) / bsdfSample->pdf;
        ray = Ray(inct.pos, bsdfSample->wi, EPS);
    }

    return ret;
}

} // namespace Atrc
