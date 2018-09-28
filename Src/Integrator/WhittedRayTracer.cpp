#include "WhittedRayTracer.h"

AGZ_NS_BEG(Atrc)

Spectrum WhittedRayTracer::SingleLightIllumination(
    const Scene &scene, const Ray &r, const Light &light,
    const EntityIntersection &inct) const
{
    uint32_t sampleCount = light.SampleCount(lightSampleLevel_);

    for(uint32_t i = 0; i < sampleCount; ++i)
    {
        // TODO
    }

    return SPECTRUM::BLACK;
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

WhittedRayTracer::WhittedRayTracer(uint32_t lightSampleLevel, uint32_t maxDepth)
    : lightSampleLevel_(lightSampleLevel), maxDepth_(maxDepth)
{
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
