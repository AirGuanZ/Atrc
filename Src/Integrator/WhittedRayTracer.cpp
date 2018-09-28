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
        LightSample lightSample = light.SampleTo(
            inct.geoInct.local.position, *lightSamSeq_);

        // Shadow ray test
        Vec3r wi = lightSample.dst2pos.Normalize();
        Real dis = (inct.geoInct.local.position - lightSample.pos).Length();
        Ray visibilityTestRay = Ray::NewSegment(
            inct.geoInct.local.position, wi,
            Real(1e-5), dis - Real(1e-5));
        if(HasIntersection(scene, visibilityTestRay))
            continue;

        // Cosine factor (lambertain law)
        auto cosFactor = SpectrumScalar(Dot(inct.geoInct.local.normal, wi));
        if(cosFactor <= SpectrumScalar(0))
            continue;

        Vec3r wo = -r.direction.Normalize();
        ret += inct.bxdf->Eval(wi, wo) * cosFactor
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
    // TODO
    return Spectrum();
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
         + IndirectIllumination(scene, r, inct.value(), depth);
}

WhittedRayTracer::WhittedRayTracer(
    SampleSeq2D *lightSamSeq, uint32_t lightSampleLevel, uint32_t maxDepth)
    : lightSamSeq_(lightSamSeq), lightSampleLevel_(lightSampleLevel), maxDepth_(maxDepth)
{
    AGZ_ASSERT(lightSamSeq);
    if(lightSampleLevel < 1)
        throw ArgumentException("WhittedRayTracer: lightSampleLevel must be non-zero");
    if(maxDepth <= 0)
        throw ArgumentException("WhittedRayTracer: maxDepth must be non-zero");
}

Spectrum WhittedRayTracer::GetRadiance(const Scene &scene, const Ray &r) const
{
    AGZ_ASSERT(maxDepth > 0);
    return Trace(scene, r, 1);
}

AGZ_NS_END(Atrc)
