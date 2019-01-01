#include <Atrc/Lib/Renderer/PathTracingIntegrator/DirectLighting.h>
#include <Atrc/Lib/Renderer/PathTracingIntegrator/FullPathTracingIntegrator.h>

namespace Atrc
{
    
FullPathTracingIntegrator::FullPathTracingIntegrator(int minDepth, int maxDepth, Real contProb, bool sampleAllLights) noexcept
    : minDepth_(minDepth), maxDepth_(maxDepth), contProb_(contProb), sampleAllLights_(sampleAllLights)
{
    AGZ_ASSERT(0 < minDepth && minDepth <= maxDepth && 0 < contProb && contProb <= 1);
}

Spectrum FullPathTracingIntegrator::Eval(const Scene &scene, const Ray &_r, Sampler *sampler, Arena &arena) const
{
    Spectrum coef(Real(1)), ret;
    Ray r = _r.Normalize();

    bool hasInct = false;
    Intersection inct;

    for(int depth = 1; depth <= maxDepth_; ++depth)
    {
        if(depth > minDepth_)
        {
            if(sampler->GetReal() > contProb_)
                break;
            coef /= contProb_;
        }

        if(coef.r < EPS && coef.g < EPS && coef.b < EPS)
            break;

        if(depth == 1)
            hasInct = scene.FindIntersection(r, &inct);

        auto med = hasInct ? inct.mediumInterface.GetMedium(inct.coordSys.ez, -r.d)
                           : scene.GetGlobalMedium();

        if(depth == 1)
        {
            if(hasInct)
            {
                auto light = inct.entity->AsLight();
                if(light)
                {
                    auto tr = med ? med->Tr(r.o, inct.pos) : Spectrum(Real(1));
                    ret += tr * coef * light->AreaLe(inct);
                }
            }
            else if(!med)
            {
                for(auto light : scene.GetLights())
                    ret += coef * light->NonAreaLe(r);
                break;
            }
        }

        bool sampleMed = false;
        Real sampleInctPDF = 1;
        MediumShadingPoint mshd; ShadingPoint shd;
        MediumPoint mpnt;

        if(med)
        {
            Real sampleMedT1 = hasInct ? (inct.t - EPS) : RealT::Infinity().Value();
            Ray sampleMedRay(r.o, r.d, EPS, Max(EPS, sampleMedT1));
            auto tMedSample = med->SampleLs(sampleMedRay, sampler->GetReal3());

            if(auto medSample = std::get_if<Medium::MediumLsSample>(&tMedSample))
            {
                mpnt = medSample->pnt;
                mshd = med->GetShadingPoint(mpnt, arena);
                coef *= med->Tr(mpnt.pos, sampleMedRay.o) / medSample->pdf;
                ret += coef * mshd.le;
                coef *= mshd.sigmaS;
                sampleMed = true;
            }
            else
                sampleInctPDF = std::get<Real>(tMedSample);
        }

        if(!sampleMed)
        {
            if(!hasInct)
                break;
            shd = inct.material->GetShadingPoint(inct, arena);
            coef *= (med ? med->Tr(r.o, inct.pos) : Spectrum(Real(1))) / sampleInctPDF;
        }

        if(sampleMed)
        {
            auto [dL, phSample, nInct] = ComputeDirectLighting(scene, mpnt, mshd, sampleAllLights_, sampler);
            ret += coef * dL;
            r = Ray(mpnt.pos, phSample.wi.Normalize(), EPS);
            if(nInct)
            {
                hasInct = true;
                inct = *nInct;
            }
            else
                hasInct = false;
        }
        else
        {
            auto [dL, bsdfSample, nInct] = ComputeDirectLighting(scene, inct, shd, sampleAllLights_, true, sampler);
            ret += coef * dL;
            if(!bsdfSample)
                break;
            coef *= bsdfSample->coef * Abs(Cos(bsdfSample->wi, shd.coordSys.ez)) / bsdfSample->pdf;
            r = Ray(inct.pos, bsdfSample->wi.Normalize(), EPS);
            if(nInct)
            {
                hasInct = true;
                inct = *nInct;
            }
            else
                hasInct = false;
        }
    }

    return ret;
}

} // namespace Atrc
