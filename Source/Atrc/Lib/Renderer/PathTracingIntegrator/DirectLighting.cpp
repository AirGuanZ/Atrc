#include <Atrc/Lib/Core/Sampler.h>
#include <Atrc/Lib/Renderer/PathTracingIntegrator/DirectLighting.h>

namespace Atrc
{

namespace
{
    Spectrum MISSampleLight(
        const Scene &scene, const Light *light,
        const Intersection &inct, const ShadingPoint &shd,
        bool useMIS, bool considerMedium, const Vec3 &sample)
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
            if(auto med = inct.mediumInterface.GetMedium(inct.coordSys.ez, lightSample.wi))
            {
                if(lightSample.isInf)
                    tr = med->TrToInf(inct.pos, lightSample.wi);
                else
                    tr = med->Tr(inct.pos, lightSample.pos);
            }
        }

        f *= tr * Abs(Cos(lightSample.wi, shd.coordSys.ez)) * lightSample.radiance;

        if(lightSample.isDelta || !useMIS)
            return f / lightSample.pdf;

        constexpr auto bsdfType = BSDFType(BSDF_ALL & ~BSDF_SPECULAR);

        Real bpdf = shd.bsdf->SampleWiPDF(shd.coordSys, inct.coordSys, lightSample.wi, inct.wr, bsdfType, false);
        return f / (lightSample.pdf + bpdf);
    }
}
    
std::pair<Spectrum, Option<BSDF::SampleWiResult>> ComputeDirectLighting(
    const Scene &scene, const Intersection &inct, const ShadingPoint &shd,
    bool sampleAllLights, bool useMIS, bool considerMedium, Sampler *sampler)
{
    Spectrum ret;

    if(sampleAllLights)
    {
        for(auto light : scene.GetLights())
            ret += MISSampleLight(scene, light, inct, shd, useMIS, considerMedium, sampler->GetReal3());
    }
    else if(auto selectLight = scene.SampleLight(sampler->GetReal()))
    {
        ret += 1 / selectLight->pdf * MISSampleLight(
            scene, selectLight->light, inct, shd, useMIS, considerMedium, sampler->GetReal3());
    }

    auto bsdfSample = shd.bsdf->SampleWi(shd.coordSys, inct.coordSys, inct.wr, BSDF_ALL, false, sampler->GetReal2());
    if(!bsdfSample || !bsdfSample->coef)
        return { ret, None };

    if(!useMIS)
        return { ret, bsdfSample };

    Ray nR(inct.pos, bsdfSample->wi, EPS);

    Real selectLightSample = sampler->GetReal();

    Intersection nInct;
    if(scene.FindIntersection(nR, &nInct))
    {
        auto light = nInct.entity->AsLight();
        if(light)
        {
            Spectrum tr(Real(1));
            if(considerMedium)
            {
                if(auto med = nInct.mediumInterface.GetMedium(nInct.coordSys.ez, nInct.wr))
                    tr = med->Tr(inct.pos, nInct.pos);
            }

            auto f = tr * bsdfSample->coef * Abs(Cos(bsdfSample->wi, shd.coordSys.ez)) * light->AreaLe(nInct);

            if(bsdfSample->isDelta || !useMIS)
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
            if(auto med = nInct.mediumInterface.GetMedium(inct.coordSys.ez, nR.d))
                tr = med->TrToInf(nR.o, nR.d);
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

    return { ret, bsdfSample };
}

} // namespace Atrc
