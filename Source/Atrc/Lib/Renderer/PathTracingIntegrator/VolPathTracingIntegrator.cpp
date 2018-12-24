#include <Atrc/Lib/Renderer/PathTracingIntegrator/VolPathTracingIntegrator.h>

namespace Atrc
{
    
VolPathTracingIntegrator::VolPathTracingIntegrator(
    int minDepth, int maxDepth, Real contProb, bool sampleAllLights, bool lightInMedium) noexcept
    : minDepth_(minDepth), maxDepth_(maxDepth), contProb_(contProb),
      sampleAllLights_(sampleAllLights), lightInMedium_(lightInMedium)
{

}

namespace
{
    Spectrum Tr(const Medium *med, const Vec3 &a, const Vec3 &b) { return med ? med->Tr(a, b) : Spectrum(Real(1)); }
    Spectrum Tr2Inf(const Medium *med, const Vec3 &a, const Vec3 &d) { return med ? med->TrToInf(a, d) : Spectrum(Real(1)); }

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

        constexpr auto bsdfType = BSDFType(BSDF_ALL & ~BSDF_SPECULAR);

        Real bpdf = shd.bsdf->SampleWiPDF(shd.coordSys, inct.coordSys, lightSample.wi, inct.wr, bsdfType);
        return f / (lightSample.pdf + bpdf);
    }

    Spectrum MISSampleLight(
        const Medium *med, const Scene &scene, const Light *light,
        const MediumPoint &mpnt, const MediumShadingPoint &mshd, const Vec3 &sample)
    {
        auto lightSample = light->SampleWi(mpnt.pos, sample);
        if(!lightSample.radiance || !lightSample.pdf)
            return Spectrum();

        Ray shadowRay(mpnt.pos, lightSample.wi.Normalize(), EPS, (lightSample.pos - mpnt.pos).Length() - EPS);
        if(scene.HasIntersection(shadowRay))
            return Spectrum();

        auto eval = mshd.ph->Eval(lightSample.wi, mpnt.wo);
        if(!eval)
            return Spectrum();

        auto tr = med ? (lightSample.isInf ? med->TrToInf(mpnt.pos, lightSample.wi)
                                           : med->Tr(mpnt.pos, lightSample.pos))
                      : Spectrum(Real(1));
        auto f = tr * eval * lightSample.radiance;

        if(lightSample.isDelta)
            return f / lightSample.pdf;

        return f / (lightSample.pdf + eval);
    }
}

Spectrum VolPathTracingIntegrator::Eval(const Scene &scene, const Ray &_r, Sampler *sampler, Arena &arena) const
{
    Spectrum coef = Spectrum(Real(1)), ret;
    Intersection inct;
    Ray r = _r;

    bool isInctValid = scene.FindIntersection(r, &inct);
    auto initMed = isInctValid ? inct.mediumInterface.GetMedium(inct.coordSys.ez, -r.d)
                               : scene.GetGlobalMedium();
    if(isInctValid)
    {
        auto light = inct.entity->AsLight();
        if(light)
            ret += coef * Tr(initMed, r.o, inct.pos) * light->AreaLe(inct);
    }
    else
    {
        auto tr = Tr2Inf(initMed, r.o, r.d);
        for(auto lht : scene.GetLights())
            ret += coef * tr * lht->NonAreaLe(r);
    }

    for(int depth = 0; depth <= maxDepth_; ++depth)
    {
        if(depth > minDepth_)
        {
            if(sampler->GetReal() > contProb_)
                break;
            coef /= contProb_;
        }

        if(coef.r < EPS && coef.g < EPS && coef.b < EPS)
            break;

        auto med = isInctValid ? inct.mediumInterface.GetMedium(inct.coordSys.ez, -r.d)
                               : scene.GetGlobalMedium();

        // 采样medium和inct

        bool sampleMed = false; // 采样点是否落在了介质当中
        Real sampleInctPDF = 1; // 如果采样了inct（即!sampleMed），那么这一行为的PDF是多少
        MediumShadingPoint mshd; ShadingPoint shd;
        MediumPoint mpnt;

        if(med)
        {
            // 构造用来采样介质/inct的ray

            Real sampleMedT1 = isInctValid ? (inct.t - EPS) : RealT::Infinity().Value();
            Ray sampleMedRay(r.o, r.d.Normalize(), EPS, Max(EPS, sampleMedT1));
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
            if(!isInctValid)
                break;
            shd = inct.entity->GetMaterial(inct)->GetShadingPoint(inct, arena);
            coef *= Tr(med, r.o, inct.pos) / sampleInctPDF;
        }

        // 采样光源以计算直接光照

        if(sampleAllLights_)
        {
            if(sampleMed)
            {
                if(lightInMedium_)
                {
                    for(auto light : scene.GetLights())
                        ret += coef * MISSampleLight(med, scene, light, mpnt, mshd, sampler->GetReal3());
                }
            }
            else
            {
                for(auto light : scene.GetLights())
                    ret += coef * MISSampleLight(med, scene, light, inct, shd, sampler->GetReal3());
            }
        }
        else if(auto light = scene.SampleLight(sampler->GetReal()))
        {
            if(sampleMed)
            {
                ret += coef * MISSampleLight(
                    med, scene, light->light, mpnt, mshd, sampler->GetReal3()) / light->pdf;
            }
            else
            {
                ret += coef * MISSampleLight(
                    med, scene, light->light, inct, shd, sampler->GetReal3()) / light->pdf;
            }
        }

        // 采样bsdf/ph以构造下一条ray nR

        Vec3 nPos = sampleMed ? mpnt.pos : inct.pos;
        Vec3 nDir; Spectrum nCoef;
        bool isNDirDelta = false;
        Real nDirPDF;

        if(sampleMed)
        {
            auto phSample = mshd.ph->SampleWi(sampler->GetReal2());
            nDir    = phSample.wi.Normalize();
            nCoef   = Spectrum(phSample.coef);
            nDirPDF = phSample.coef;
        }
        else
        {
            auto bsdfSample = shd.bsdf->SampleWi(
                shd.coordSys, inct.coordSys, inct.wr, BSDF_ALL, sampler->GetReal2());
            if(!bsdfSample)
                break;
            nDir        = bsdfSample->wi.Normalize();
            nCoef       = bsdfSample->coef * Abs(Cos(bsdfSample->wi, shd.coordSys.ez));;
            nDirPDF     = bsdfSample->pdf;
            isNDirDelta = bsdfSample->isDelta;
        }

        Ray nR(nPos, nDir, EPS);

        // 求nR与场景的交点，以计算MIS下的直接光照
        // 并更新r与inct

        Intersection nInct;
        if(scene.FindIntersection(nR, &nInct))
        {
            if(!sampleMed || lightInMedium_)
            {
                auto nMed = nInct.mediumInterface.GetMedium(nInct.coordSys.ez, nInct.wr);

                auto light = nInct.entity->AsLight();
                if(light)
                {
                    auto tr = Tr(nMed, nR.o, nInct.pos);
                    Real lpdf = sampleAllLights_ ? Real(1) : scene.SampleLightPDF(light);

                    if(isNDirDelta)
                        ret += tr * coef * nCoef * light->AreaLe(nInct) / nDirPDF;
                    else
                    {
                        if(sampleMed)
                            lpdf *= light->SampleWiAreaPDF(nInct.pos, nInct.coordSys.ez, mpnt.pos);
                        else
                            lpdf *= light->SampleWiAreaPDF(nInct.pos, nInct.coordSys.ez, inct, shd);
                        ret += tr * coef * nCoef * light->AreaLe(nInct) / (nDirPDF + lpdf);
                    }
                }
            }

            coef *= nCoef / nDirPDF;
            isInctValid = true;
            r    = nR;
            inct = nInct;
        }
        else
        {
            if(!sampleMed || lightInMedium_)
            {
                auto tr = Tr2Inf(scene.GetGlobalMedium(), nR.o, nR.d);

                if(sampleAllLights_)
                {
                    for(auto lht : scene.GetLights())
                    {
                        auto le = lht->NonAreaLe(nR);
                        if(!le)
                            continue;
                        if(isNDirDelta)
                            ret += coef * tr * nCoef * le / nDirPDF;
                        else
                        {
                            Real lpdf;
                            if(sampleMed)
                                lpdf = lht->SampleWiNonAreaPDF(nR.d, mpnt.pos);
                            else
                                lpdf = lht->SampleWiNonAreaPDF(nR.d, inct, shd);
                            ret += coef * tr * nCoef * le / (nDirPDF + lpdf);
                        }
                    }
                }
                else if(auto lht = scene.SampleLight(sampler->GetReal()))
                {
                    auto le = lht->light->NonAreaLe(nR);
                    if(!!le)
                    {
                        if(isNDirDelta)
                            ret += coef * tr * nCoef * le / nDirPDF;
                        else
                        {
                            Real lpdf = lht->pdf;
                            if(sampleMed)
                                lpdf *= lht->light->SampleWiNonAreaPDF(nR.d, mpnt.pos);
                            else
                                lpdf *= lht->light->SampleWiNonAreaPDF(nR.d, inct, shd);
                            ret += coef * tr * nCoef * le / (nDirPDF + lpdf);
                        }
                    }
                }
            }

            coef *= nCoef / nDirPDF;
            isInctValid = false;
            r = nR;
        }
    }

    return ret;
}


} // namespace Atrc
