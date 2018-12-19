#include <Atrc/Lib/Renderer/PathTracingIntegrator/MISPathTracingIntegrator.h>

namespace Atrc
{

Spectrum MISPathTracingIntegrator::MISSampleLight(
    const Scene &scene, const Light *light,
    const Intersection &inct, const ShadingPoint &shd, const Vec3 &sample) const
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
    f *= Abs(Cos(lightSample.wi, shd.coordSys.ez));

    if(lightSample.isDelta)
        return f * lightSample.radiance / lightSample.pdf;

    constexpr BSDFType bsdfType = BSDFType(BSDF_ALL & ~BSDF_SPECULAR);

    Real bpdf = shd.bsdf->SampleWiPDF(shd.coordSys, inct.coordSys, lightSample.wi, inct.wr, bsdfType);
    return f * lightSample.radiance / (lightSample.pdf + bpdf);
}

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
    Spectrum coef = Spectrum(Real(1)), ret;
    Ray ray = r;

    Intersection inct;
    if(scene.FindIntersection(ray, &inct))
    {
        if(auto light = inct.entity->AsLight())
            ret += light->AreaLe(inct); // NonareaLe must be BLACK
    }
    else
    {
        Spectrum le;
        for(auto light : scene.GetLights())
            le += light->NonAreaLe(ray); // AreaLe must be BLACK
        return le;
    }

    for(int i = 1; i <= maxDepth_; ++i)
    {
        // Russian roulette strategy

        if(i > minDepth_)
        {
            if(sampler->GetReal() > contProb_)
                break;
            coef /= contProb_;
        }

        // BSDF

        ShadingPoint shd = inct.entity->GetMaterial(inct)
                                      ->GetShadingPoint(inct, arena);

        // Sample direct illumination with MIS

        if(sampleAllLights_)
        {
            for(auto light : scene.GetLights())
                ret += coef * MISSampleLight(scene, light, inct, shd, sampler->GetReal3());
        }
        else
        {
            auto light = scene.SampleLight(sampler->GetReal());
            if(light)
            {
                ret += coef * MISSampleLight(
                    scene, light->light, inct, shd, sampler->GetReal3()) / light->pdf;
            }
        }

        auto bsdfSample = shd.bsdf->SampleWi(
            shd.coordSys, inct.coordSys, inct.wr, BSDF_ALL, sampler->GetReal2());
        if(bsdfSample && !!bsdfSample->coef)
        {
            Spectrum f = bsdfSample->coef * Abs(Cos(bsdfSample->wi, shd.coordSys.ez));
            ray = Ray(inct.pos, bsdfSample->wi, EPS);
            Intersection tInct;

            if(scene.FindIntersection(ray, &tInct))
            {
                auto light = tInct.entity->AsLight();
                if(light)
                {
                    if(bsdfSample->isDelta)
                        ret += coef * f * light->AreaLe(tInct) / bsdfSample->pdf;
                    else
                    {
                        Real lpdf = scene.SampleLightPDF(light) *
                            light->SampleWiAreaPDF(tInct.pos, tInct.coordSys.ez, inct, shd);
                        ret += coef * f * light->AreaLe(tInct) / (bsdfSample->pdf + lpdf);
                    }
                }

                coef *= f / bsdfSample->pdf;
                ray = Ray(inct.pos + EPS * inct.coordSys.ez, bsdfSample->wi, EPS);
                inct = tInct;
            }
            else
            {
                if(sampleAllLights_)
                {
                    for(auto light : scene.GetLights())
                    {
                        auto le = light->NonAreaLe(r);
                        if(!le)
                            continue;
                        if(bsdfSample->isDelta)
                            ret += coef * f * le / bsdfSample->pdf;
                        else
                        {
                            Real lpdf = light->SampleWiNonAreaPDF(ray.d, inct, shd);
                            ret += coef * f * le / (bsdfSample->pdf + lpdf);
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
                        ret += coef * f * le / bsdfSample->pdf;
                    else
                    {
                        Real lpdf = light->pdf * light->light->SampleWiNonAreaPDF(ray.d, inct, shd);
                        ret += coef * f * le / (bsdfSample->pdf + lpdf);
                    }
                }

                break;
            }
        }
        else
            break;
    }

    return ret;
}

} // namespace Atrc
