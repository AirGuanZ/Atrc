#include <Atrc/Lib/Core/BSSRDF.h>
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

    Intersection inct;
    bool hasInct = false;

    for(int depth = 1; depth <= maxDepth_; ++depth)
    {
        if(depth > minDepth_)
        {
            if(sampler->GetReal() > contProb_)
                break;
            coef /= contProb_;
        }

        if(depth == 1)
        {
            hasInct = scene.FindIntersection(ray, &inct);
            if(hasInct)
            {
                if(auto light = inct.entity->AsLight())
                    ret += coef * light->AreaLe(inct);
            }
            else
            {
                for(auto light : scene.GetLights())
                    ret += coef * light->NonAreaLe(ray);
            }
        }

        if(!hasInct)
            break;

        auto shd = inct.material->GetShadingPoint(inct, arena);
        auto [directLighting, bsdfSample, nInct] = ComputeDirectLighting(scene, inct, shd, sampleAllLights_, false, sampler);
        ret += coef * directLighting;

        if(!bsdfSample)
            break;

        coef *= bsdfSample->coef * Abs(Cos(bsdfSample->wi, shd.coordSys.ez)) / bsdfSample->pdf;
        ray = Ray(inct.pos, bsdfSample->wi.Normalize(), EPS);

        if(shd.bssrdf && bsdfSample->type & BSDF_TRANSMISSION)
        {
            auto piSample = shd.bssrdf->SamplePi(false, sampler->GetReal3(), arena);
            if(!piSample)
                break;

            auto deltaCoef = piSample->coef / piSample->pdf;
            coef *= deltaCoef;

            AGZ_ASSERT(!piSample->coef.HasInf());

            auto pishd = piSample->pi.material->GetShadingPoint(piSample->pi, arena);
            AGZ_ASSERT(!pishd.bssrdf);

            auto [piDirL, piBsdfSample, piNInct] = ComputeDirectLighting(
                scene, piSample->pi, pishd, sampleAllLights_, false, sampler);
            ret += coef * piDirL;

            AGZ_ASSERT(!coef.HasInf());
            AGZ_ASSERT(!piDirL.HasInf());
            AGZ_ASSERT(!ret.HasInf());

            if(!piBsdfSample)
                break;

            deltaCoef = piBsdfSample->coef / piBsdfSample->pdf* Abs(Cos(piBsdfSample->wi, pishd.coordSys.ez));

            AGZ_ASSERT(Dot(piBsdfSample->wi, pishd.coordSys.ez) >= 0);
            coef *= deltaCoef;
            ray = Ray(piSample->pi.pos, piBsdfSample->wi.Normalize(), EPS);
            nInct = piNInct;
        }

        if(nInct)
        {
            inct = *nInct;
            hasInct = true;
        }
        else
            hasInct = false;
    }

    return ret;
}

} // namespace Atrc
