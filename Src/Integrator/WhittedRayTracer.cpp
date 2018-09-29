#include "WhittedRayTracer.h"

AGZ_NS_BEG(Atrc)

Spectrum WhittedRayTracer::SingleLightIllumination(
    const Scene &scene, const Ray &r, const Light &light,
    const EntityIntersection &inct) const
{
    Spectrum ret = SPECTRUM::BLACK;
    uint32_t sampleCount = light.SampleCount(lightSampleLevel_);

    for(uint32_t i = 0; i < sampleCount; ++i)
    {
        Vec3r newOri = inct.geoInct.local.position +
                       Real(1e-5) * inct.geoInct.local.normal;

        LightSample lightSample = light.SampleTo(
            newOri, *lightSamSeq_);

        // Shadow ray test
        Vec3r wi = lightSample.dst2pos.Normalize();
        Real dis = lightSample.dst2pos.Length();
        Ray shadowRay = Ray::NewSegment(
            newOri, wi, Real(1e-5), dis - Real(1e-5));
        if(HasIntersection(scene, shadowRay))
            continue;

        // Cosine factor (lambertain law)
        auto cosFactor = SpectrumScalar(Dot(inct.geoInct.local.normal, wi));
        if(cosFactor <= SpectrumScalar(0))
            continue;

        Vec3r wo = -r.direction.Normalize();
        Spectrum bxdfVal = inct.bxdf->Eval(wi, wo);
        if(bxdfVal == SPECTRUM::BLACK)
            continue;

        ret += bxdfVal * cosFactor
             * lightSample.spectrum / SpectrumScalar(lightSample.pdf);
    }

    return ret / sampleCount;
}

Spectrum WhittedRayTracer::DirectIllumination(
    const Scene &scene, const Ray &r, const EntityIntersection &inct) const
{
    Spectrum ret = SPECTRUM::BLACK;
    for(auto light : scene.GetLights())
        ret += SingleLightIllumination(scene, r, *light, inct);
    return ret;
}

Spectrum WhittedRayTracer::IndirectIllumination(
    const Scene &scene, const Ray &r, const EntityIntersection &inct,
    uint32_t depth) const
{
    Vec3r wo = -r.direction.Normalize();

    // reflection
    auto refSample = inct.bxdf->Sample(inct.geoInct.local, wo, *lightSamSeq_,
                                       CombineBxDFTypes(BxDFType::Reflection, BxDFType::Specular));
    if(!refSample)
        return SPECTRUM::BLACK;

    Ray refRay = Ray::NewSegment(inct.geoInct.local.position, refSample->dir.Normalize(), Real(1e-5));
    auto cosFactor = SpectrumScalar(Dot(refRay.direction, inct.geoInct.local.normal));

    return Trace(scene, refRay, depth + 1) * cosFactor * refSample->coef / SpectrumScalar(refSample->pdf);
}

Spectrum WhittedRayTracer::Trace(
    const Scene &scene, const Ray &r, uint32_t depth) const
{
    if(depth > maxDepth_)
        return SPECTRUM::BLACK;

    auto inct = EvalIntersection(scene, r);
    if(!inct)
        return SPECTRUM::BLACK;

    return DirectIllumination(scene, r, inct.value())
         + IndirectIllumination(scene, r, inct.value(), depth)
         + inct->bxdf->AmbientRadiance();
}

WhittedRayTracer::WhittedRayTracer(
    SampleSeq2D &lightSamSeq, uint32_t lightSampleLevel, uint32_t maxDepth)
    : lightSamSeq_(&lightSamSeq), lightSampleLevel_(lightSampleLevel), maxDepth_(maxDepth)
{
    if(lightSampleLevel < 1)
        throw ArgumentException("WhittedRayTracer: lightSampleLevel must be non-zero");
    if(maxDepth <= 0)
        throw ArgumentException("WhittedRayTracer: maxDepth must be non-zero");
}

Spectrum WhittedRayTracer::GetRadiance(const Scene &scene, const Ray &r) const
{
    AGZ_ASSERT(maxDepth_ > 0);
    return Trace(scene, r, 1);
}

AGZ_NS_END(Atrc)
