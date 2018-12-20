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

namespace
{
    struct TrLeLeftResult
    {
        Spectrum trle;
        bool isInctValid;
    };

    TrLeLeftResult TrLeLeft(const Scene &scene, const Ray &r, Intersection *inct)
    {
        bool isInctValid = scene.FindIntersection(r, inct);

        if(isInctValid)
        {
            auto light = inct->entity->AsLight();
            if(!light)
                return { Spectrum(), true };
                
            auto med = inct->mediumInterface.GetMedium(inct->coordSys.ez, -r.d);
            Spectrum tr = med ? med->Tr(r.o, inct->pos) : Spectrum(Real(1));

            return { tr * light->AreaLe(*inct), true };
        }

        Spectrum spec;
        auto gMed = scene.GetGlobalMedium();
        auto tr = gMed ? gMed->TrToInf(r.o, r.d) : Spectrum(Real(1));
        for(auto lht : scene.GetLights())
            spec += tr * lht->NonAreaLe(r);

        return { spec, false };
    }

    Spectrum MISSampleLight(
        const Medium *med, const Scene &scene, const Light *light,
        const Intersection &inct, const ShadingPoint &shd, const Vec3 &sample)
    {
        auto lightSample = light->SampleWi(inct, shd, sample);
        if(!lightSample.radiance || !lightSample.pdf)
            return Spectrum();
                    
        Ray shadowRay(inct.pos, lightSample.wi.Normalize(), EPS, (lightSample.pos - inct.pos).Length() - EPS);
        if(scene.HasIntersection(shadowRay))
            return Spectrum();
                    
        auto f = shd.bsdf->Eval(shd.coordSys, inct.coordSys, lightSample.wi, inct.wr, BSDF_ALL);
        if(!f)
            return Spectrum();

        auto tr = med ? (lightSample.isInf ? med->TrToInf(inct.pos, lightSample.wi)
                                           : med->Tr(inct.pos, lightSample.pos))
                      : Spectrum(Real(1));
        f *= tr * Abs(Cos(lightSample.wi, shd.coordSys.ez)) * lightSample.radiance;

        if(lightSample.isDelta)
            return f / lightSample.pdf;

        constexpr BSDFType bsdfType = BSDFType(BSDF_ALL & ~BSDF_SPECULAR);

        Real bpdf = shd.bsdf->SampleWiPDF(shd.coordSys, inct.coordSys, lightSample.wi, inct.wr, bsdfType);
        return f / (lightSample.pdf + bpdf);
    }
}

Spectrum VolumetricPathTracingIntegrator::Eval(const Scene &scene, const Ray &_r, Sampler *sampler, Arena &arena) const
{
    // Invariant: keep r normalized

    Spectrum coef = Spectrum(Real(1));
    Ray r = _r.Normalize();

    Intersection inct;
    auto [ret, isInctValid] = TrLeLeft(scene, r, &inct);

    for(int depth = 1; depth <= maxDepth_; ++depth)
    {
        // Russian roulette strategy

        if(depth > minDepth_)
        {
            if(sampler->GetReal() > contProb_)
                break;
            coef /= contProb_;
        }

        Ray newRay = r;

        Real sampleInctPDF = 1; // 如果没有采样介质中的点，那么选择采样inct的pdf

        // 尝试采样介质中的点

        auto med = isInctValid ? inct.mediumInterface.GetMedium(inct.coordSys.ez, -r.d)
                               : scene.GetGlobalMedium();
        if(med)
        {
            Ray medSampleRay = r;
            Real rt1 = isInctValid ? (inct.t - EPS) : RealT::Infinity().Value();
            medSampleRay.t1 = Max(rt1, r.t0);
            auto tMedSample = med->SampleLs(medSampleRay, sampler->GetReal3());

            if(auto medSample = std::get_if<Medium::MediumLsSample>(&tMedSample))
            {
                auto tr = med->Tr(medSample->pnt.pos, medSampleRay.o);
                coef *= tr / medSample->pdf;

                auto shd = med->GetShadingPoint(medSample->pnt, arena);
                ret += coef * shd.le;

                coef *= shd.sigmaS;

                auto phSample = shd.ph->SampleWi(sampler->GetReal2());
                r =  Ray(medSample->pnt.pos, phSample.wi.Normalize(), EPS);

                auto trLeftRt = TrLeLeft(scene, r, &inct);
                ret += coef * trLeftRt.trle;
                isInctValid = trLeftRt.isInctValid;

                continue;
            }
            
            sampleInctPDF = std::get<Real>(tMedSample);
        }

        if(!isInctValid)
            break;

        // 采样inct

        auto tr = med ? med->Tr(r.o, inct.pos) : Spectrum(Real(1));
        coef *= tr;

        ShadingPoint shd = inct.entity->GetMaterial(inct)->GetShadingPoint(inct, arena);

        if(sampleAllLights_)
        {
            for(auto light : scene.GetLights())
                ret += coef * MISSampleLight(med, scene, light, inct, shd, sampler->GetReal3());
        }
        else
        {
            auto light = scene.SampleLight(sampler->GetReal());
            if(light)
            {
                ret += coef * MISSampleLight(
                    med, scene, light->light, inct, shd, sampler->GetReal3()) / light->pdf;
            }
        }

        auto bsdfSample = shd.bsdf->SampleWi(
            shd.coordSys, inct.coordSys, inct.wr, BSDF_ALL, sampler->GetReal2());
        if(bsdfSample && !!bsdfSample->coef)
        {
            Spectrum f = bsdfSample->coef * Abs(Cos(bsdfSample->wi, shd.coordSys.ez));
            r = Ray(inct.pos, bsdfSample->wi.Normalize(), EPS);
            Intersection tInct;

            if(scene.FindIntersection(r, &tInct))
            {
                auto light = tInct.entity->AsLight();
                if(light)
                {
                    auto tr = med ? med->Tr(r.o, tInct.pos) : Spectrum(Real(1));

                    if(bsdfSample->isDelta)
                        ret += tr * coef * f * light->AreaLe(tInct) / bsdfSample->pdf;
                    else
                    {
                        Real lpdf = scene.SampleLightPDF(light) *
                            light->SampleWiAreaPDF(tInct.pos, tInct.coordSys.ez, inct, shd);
                        ret += tr * coef * f * light->AreaLe(tInct) / (bsdfSample->pdf + lpdf);
                    }
                }

                coef *= f / bsdfSample->pdf;
                isInctValid = true;
                inct = tInct;
            }
            else
            {
                isInctValid = false;
                auto tr = med ? med->TrToInf(r.o, r.d) : Spectrum(Real(1));

                if(sampleAllLights_)
                {
                    for(auto light : scene.GetLights())
                    {
                        auto le = light->NonAreaLe(r);
                        if(!le)
                            continue;
                        if(bsdfSample->isDelta)
                            ret += tr * coef * f * le / bsdfSample->pdf;
                        else
                        {
                            Real lpdf = light->SampleWiNonAreaPDF(r.d, inct, shd);
                            ret += tr * coef * f * le / (bsdfSample->pdf + lpdf);
                        }
                    }
                }
                else
                {
                    auto light = scene.SampleLight(sampler->GetReal());
                    if(!light)
                        break;

                    auto le = light->light->NonAreaLe(r);
                    if(!le)
                        break;

                    if(bsdfSample->isDelta)
                        ret += tr * coef * f * le / bsdfSample->pdf;
                    else
                    {
                        Real lpdf = light->pdf * light->light->SampleWiNonAreaPDF(r.d, inct, shd);
                        ret += tr * coef * f * le / (bsdfSample->pdf + lpdf);
                    }
                }
            }
        }
        else
            break;
    }

    return ret;
}

} // namespace Atrc
