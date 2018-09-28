#include "WhittedRayTracer.h"

AGZ_NS_BEG(Atrc)

Spectrum WhittedRayTracer::Trace(const Scene &scene, const Ray &r, uint32_t depth) const
{
    if(depth > maxDepth_)
        return SPECTRUM::BLACK;

    auto inct = EvalIntersection(scene, r);
    if(!inct)
        return SPECTRUM::BLACK;

    // TODO
    return SPECTRUM::RED;
}

WhittedRayTracer::WhittedRayTracer(uint32_t maxDepth)
    : maxDepth_(maxDepth)
{
    if(maxDepth <= 0)
        throw ArgumentException("WhittedRayTracer: maxDepth must be non-zero");
}

Spectrum WhittedRayTracer::GetRadiance(const Scene &scene, const Ray &r) const
{
    AGZ_ASSERT(maxDepth > 0);
    return Trace(scene, r, 1);
}

AGZ_NS_END(Atrc)
