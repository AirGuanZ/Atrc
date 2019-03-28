#include <Atrc/Core/Core/Sampler.h>
#include <Atrc/Core/Renderer/PathTracingIntegrator/DirectLighting.h>

namespace Atrc
{

namespace
{
    Spectrum Tr(const Medium *med, const Vec3 &a, const Vec3 &b) { return med ? med->Tr(a, b) : Spectrum(Real(1)); }
    Spectrum Tr2Inf(const Medium *med, const Vec3 &a, const Vec3 &d) { return med ? med->TrToInf(a, d) : Spectrum(Real(1)); }

    Spectrum MISSampleLight(
        const Scene &scene, const Light *light,
        const Intersection &inct, const ShadingPoint &shd,
        bool considerMedium, const Vec3 &sample)
    {
        auto lightSample = light->SampleWi(inct, shd, sample);
        if(!lightSample.radiance || !lightSample.pdf)
            return Spectrum();

        Real shadowRayT1 = (lightSample.pos - inct.pos).Length() - EPS;
        if(shadowRayT1 <= EPS)
            return Spectrum();

        Ray shadowRay(inct.pos, lightSample.wi.Normalize(), EPS, shadowRayT1);
        if(scene.HasIntersection(shadowRay))
            return Spectrum();

        auto f = shd.bsdf->Eval(shd.coordSys, inct.coordSys, lightSample.wi, inct.wr, BSDF_ALL, false);
        if(!f)
            return Spectrum();

        auto tr = Spectrum(Real(1));
        if(considerMedium)
        {
            auto med = inct.mediumInterface.GetMedium(inct.coordSys.ez, lightSample.wi);
            if(lightSample.isInf)
                tr = Tr2Inf(med, inct.pos, lightSample.wi);
            else
                tr = Tr(med, inct.pos, lightSample.pos);
        }

        f *= tr * Abs(Cos(lightSample.wi, shd.coordSys.ez)) * lightSample.radiance;

        if(lightSample.isDelta)
            return f / lightSample.pdf;

        constexpr auto bsdfType = BSDFType(BSDF_ALL & ~BSDF_SPECULAR);

        Real bpdf = shd.bsdf->SampleWiPDF(shd.coordSys, inct.coordSys, lightSample.wi, inct.wr, bsdfType, false);
        return f / (lightSample.pdf + bpdf);
    }

    Spectrum MISSampleLight(
        const Scene &scene, const Light *light,
        const MediumPoint &mpnt, const MediumShadingPoint &mshd, const Vec3 &sample)
    {
        auto lightSample = light->SampleWi(mpnt.pos, sample);
        if(!lightSample.radiance || !lightSample.pdf)
            return Spectrum();

        Real shadowRayT1 = (lightSample.pos - mpnt.pos).Length() - EPS;
        if(shadowRayT1 <= EPS)
            return Spectrum();

        Ray shadowRay(mpnt.pos, lightSample.wi.Normalize(), EPS, shadowRayT1);
        if(scene.HasIntersection(shadowRay))
            return Spectrum();

        auto eval = mshd.ph->Eval(lightSample.wi, mpnt.wo);
        if(!eval)
            return Spectrum();

        auto tr = lightSample.isInf ? Tr2Inf(mpnt.medium, mpnt.pos, lightSample.wi)
                                    : Tr(mpnt.medium, mpnt.pos, lightSample.pos);
        auto f = eval * tr * lightSample.radiance;

        if(lightSample.isDelta)
            return f / lightSample.pdf;

        Real bpdf = eval;
        return f / (lightSample.pdf + bpdf);
    }
}
    
std::tuple<Spectrum, std::optional<BSDF::SampleWiResult>, std::optional<Intersection>> ComputeDirectLighting(
    const Scene &scene, const Intersection &inct, const ShadingPoint &shd,
    bool sampleAllLights, bool considerMedium, Sampler *sampler)
{
    Spectrum ret;

    if(sampleAllLights)
    {
        for(auto light : scene.GetLights())
            ret += MISSampleLight(scene, light, inct, shd, considerMedium, sampler->GetReal3());
    }
    else if(auto selectLight = scene.SampleLight(sampler->GetReal()))
    {
        ret += 1 / selectLight->pdf * MISSampleLight(
            scene, selectLight->light, inct, shd, considerMedium, sampler->GetReal3());
    }

    auto bsdfSample = shd.bsdf->SampleWi(shd.coordSys, inct.coordSys, inct.wr, BSDF_ALL, false, sampler->GetReal3());
    if(!bsdfSample || !bsdfSample->coef)
        return { ret, std::nullopt, std::nullopt };

    Ray nR(inct.pos, bsdfSample->wi.Normalize(), EPS);

    Real selectLightSample = sampler->GetReal();

    Intersection nInct;
    bool hasInct = scene.FindIntersection(nR, &nInct);
    if(hasInct)
    {
        auto light = nInct.entity->AsLight();
        if(light)
        {
            Spectrum tr(Real(1));
            if(considerMedium)
            {
                auto med = nInct.mediumInterface.GetMedium(nInct.coordSys.ez, nInct.wr);
                tr = Tr(med, inct.pos, nInct.pos);
            }

            auto f = tr * bsdfSample->coef * Abs(Cos(bsdfSample->wi, shd.coordSys.ez)) * light->AreaLe(nInct);

            if(bsdfSample->isDelta)
                ret += f / bsdfSample->pdf;
            else
            {
                Real lpdf = sampleAllLights ? Real(1) : scene.SampleLightPDF(light);
                lpdf *= light->SampleWiAreaPDF(nInct.pos, nInct.coordSys.ez, inct, shd);
                ret += f / (bsdfSample->pdf + lpdf);
            }
        }
    }
    else
    {
        Spectrum tr(Real(1));
        if(considerMedium)
        {
            auto med = nInct.mediumInterface.GetMedium(inct.coordSys.ez, nR.d);
            tr = Tr2Inf(med, nR.o, nR.d);
        }

        auto f = tr * bsdfSample->coef * Abs(Cos(shd.coordSys.ez, bsdfSample->wi));

        if(sampleAllLights)
        {
            for(auto light : scene.GetLights())
            {
                auto le = light->NonAreaLe(nR);
                if(!le)
                    continue;
                if(bsdfSample->isDelta)
                    ret += f * le / bsdfSample->pdf;
                else
                {
                    Real lpdf = light->SampleWiNonAreaPDF(nR.d, inct, shd);
                    ret += f * le / (bsdfSample->pdf + lpdf);
                }
            }
        }
        else if(auto lht = scene.SampleLight(selectLightSample))
        {
            auto le = lht->light->NonAreaLe(nR);
            if(!!le)
            {
                if(bsdfSample->isDelta)
                    ret += f * le / bsdfSample->pdf;
                else
                {
                    Real lpdf = lht->pdf * lht->light->SampleWiNonAreaPDF(nR.d, inct, shd);
                    ret += f * le / (bsdfSample->pdf + lpdf);
                }
            }
        }
    }

    if(hasInct)
        return std::make_tuple(ret, bsdfSample, std::make_optional(nInct));
    return { ret, std::optional(*bsdfSample), std::nullopt };
}

std::tuple<Spectrum, PhaseFunction::SampleWiResult, std::optional<Intersection>> ComputeDirectLighting(
    const Scene &scene, const MediumPoint &mpnt, const MediumShadingPoint &mshd,
    bool sampleAllLights, Sampler *sampler)
{
    Spectrum ret;

    if(sampleAllLights)
    {
        for(auto light : scene.GetLights())
            ret += MISSampleLight(scene, light, mpnt, mshd, sampler->GetReal3());
    }
    else if(auto selectLight = scene.SampleLight(sampler->GetReal()))
    {
        ret += 1 / selectLight->pdf * MISSampleLight(
            scene, selectLight->light, mpnt, mshd, sampler->GetReal3());
    }

    auto phSample = mshd.ph->SampleWi(sampler->GetReal2());
    Ray nR(mpnt.pos, phSample.wi.Normalize(), EPS);

    Real selectLightSamle = sampler->GetReal();

    Intersection nInct;
    bool hasInct = scene.FindIntersection(nR, &nInct);
    if(hasInct)
    {
        auto light = nInct.entity->AsLight();
        if(light)
        {
            Spectrum tr = Tr(mpnt.medium, mpnt.pos, nInct.pos);
            auto f = tr * phSample.coef * light->AreaLe(nInct);

            Real lpdf = sampleAllLights ? Real(1) : scene.SampleLightPDF(light);
            lpdf *= light->SampleWiAreaPDF(nInct.pos, nInct.coordSys.ez, mpnt.pos);

            ret += tr * f / (phSample.coef + lpdf);
        }
    }
    else
    {
        Spectrum tr = Tr2Inf(scene.GetGlobalMedium(), nR.o, nR.d);
        auto f = tr * phSample.coef;

        if(sampleAllLights)
        {
            for(auto light : scene.GetLights())
            {
                auto le = light->NonAreaLe(nR);
                if(!le)
                    continue;
                Real lpdf = light->SampleWiNonAreaPDF(nR.d, mpnt.pos);
                ret += f * le / (phSample.coef + lpdf);
            }
        }
        else if(auto lht = scene.SampleLight(selectLightSamle))
        {
            auto le = lht->light->NonAreaLe(nR);
            if(!!le)
            {
                Real lpdf = lht->pdf * lht->light->SampleWiNonAreaPDF(nR.d, mpnt.pos);
                ret += f * le / (phSample.coef + lpdf);
            }
        }
    }

    if(hasInct)
        return { ret, phSample, nInct };
    return { ret, phSample, std::nullopt };
}

} // namespace Atrc
