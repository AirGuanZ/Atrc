#include <Atrc/Lib/Renderer/PathTracingIntegrator/VolPathTracingIntegrator.h>

namespace Atrc
{
    
VolPathTracingIntegrator::VolPathTracingIntegrator(int minDepth, int maxDepth, Real contProb, bool sampleAllLights) noexcept
    : minDepth_(minDepth), maxDepth_(maxDepth), contProb_(contProb), sampleAllLights_(sampleAllLights)
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
                coef *= med->Tr(mpnt.pos, sampleMedRay.o) / medSample->pdf;
                mshd = med->GetShadingPoint(mpnt, arena);
                sampleMed = true;
            }
        }

        if(!sampleMed)
        {
            shd = inct.entity->GetMaterial(inct)->GetShadingPoint(inct, arena);
            coef = Tr(med, r.o, inct.pos) / sampleInctPDF;
        }

        // 采样光源以计算直接光照

        if(sampleAllLights_)
        {
            
        }
    }

    return ret;
}


} // namespace Atrc
