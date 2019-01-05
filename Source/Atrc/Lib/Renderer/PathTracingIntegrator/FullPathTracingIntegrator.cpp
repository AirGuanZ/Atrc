#include <Atrc/Lib/Core/BSSRDF.h>
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
            auto samplePnt = sampler->GetReal3();
            auto tMedSample = med->SampleLs(sampleMedRay, samplePnt);

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

        Option<Intersection> nInct;

        if(sampleMed)
        {
            auto [dL, phSample, phNInct] = ComputeDirectLighting(scene, mpnt, mshd, sampleAllLights_, sampler);
            ret += coef * dL;
            r = Ray(mpnt.pos, phSample.wi.Normalize(), EPS);
            nInct = phNInct;
        }
        else
        {
            auto [dL, bsdfSample, bsdfNInct] = ComputeDirectLighting(scene, inct, shd, sampleAllLights_, true, sampler);
            ret += coef * dL;
            if(!bsdfSample)
                break;
            coef *= bsdfSample->coef * Abs(Cos(bsdfSample->wi, shd.coordSys.ez)) / bsdfSample->pdf;
            r = Ray(inct.pos, bsdfSample->wi.Normalize(), EPS);
            nInct = bsdfNInct;

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
                AGZ_ASSERT(!ret.HasInf());;

                if(!piBsdfSample)
                    break;

                deltaCoef = piBsdfSample->coef / piBsdfSample->pdf* Abs(Cos(piBsdfSample->wi, pishd.coordSys.ez));

                AGZ_ASSERT(Dot(piBsdfSample->wi, pishd.coordSys.ez) >= 0);
                coef *= deltaCoef;
                r = Ray(piSample->pi.pos, piBsdfSample->wi.Normalize(), EPS);
                nInct = piNInct;
            }
        }

        if(nInct)
        {
            hasInct = true;
            inct = *nInct;
        }
        else
            hasInct = false;
    }

    return ret;
}

} // namespace Atrc
