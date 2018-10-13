#include <Atrc/Entity/Entity.h>
#include <Atrc/Material/BxDF.h>
#include <Atrc/Integrator/PathTracer.h>

AGZ_NS_BEG(Atrc)

Spectrum PathTracer::Trace(const Scene &scene, const Ray &r, uint32_t depth) const
{
    if(depth > maxDepth_)
        return SPECTRUM::RED;

    Intersection inct;
    if(!FindClosestIntersection(scene, r, &inct))
        return SPECTRUM::BLACK;

    auto bxdf = inct.entity->GetBxDF(inct);
    auto bxdfSample = bxdf->Sample(-r.direction, BXDF_ALL);
    if(!bxdfSample)
        return bxdf->AmbientRadiance(inct);

    Spectrum ret;
    if(inct.entity->AsLight())
        ret += inct.entity->AsLight()->Le(inct);

    auto newRay = Ray(inct.pos, bxdfSample->dir, 1e-5);
    AGZ_ASSERT(ValidDir(newRay));
    ret += bxdfSample->coef * Trace(scene, newRay, depth + 1)
         * SS(Abs(Dot(inct.nor, bxdfSample->dir)) / bxdfSample->pdf)
         + bxdf->AmbientRadiance(inct);

    return ret;
}

PathTracer::PathTracer(uint32_t maxDepth)
    : maxDepth_(maxDepth)
{
    AGZ_ASSERT(maxDepth >= 1);
}

Spectrum PathTracer::GetRadiance(const Scene &scene, const Ray &r) const
{
    return Trace(scene, r, 1);
}

AGZ_NS_END(Atrc)
