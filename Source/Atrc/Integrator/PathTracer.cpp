#include <Atrc/Entity/Entity.h>
#include <Atrc/Material/BxDF.h>
#include <Atrc/Integrator/PathTracer.h>

AGZ_NS_BEG(Atrc)

Spectrum PathTracer::Trace(const Scene &scene, const Ray &r, uint32_t depth) const
{
    float stopCoef = 1.0f;
    if(depth > minDepth_)
    {
        if(Rand() > stopProb_)
            return SPECTRUM::BLACK;
        stopCoef = stopCoef_;
    }

    Intersection inct;
    if(!FindClosestIntersection(scene, r, &inct))
        return SPECTRUM::BLACK;

    auto bxdf = inct.entity->GetBxDF(inct);
    auto bxdfSample = bxdf->Sample(-r.direction, BXDF_ALL);
    if(!bxdfSample)
        return stopCoef * bxdf->AmbientRadiance(inct);

    auto newRay = Ray(inct.pos, bxdfSample->dir, 1e-5);
    return stopCoef * 
        (bxdfSample->coef * Trace(scene, newRay, depth + 1)
       * SS(Abs(Dot(inct.nor, bxdfSample->dir)) / bxdfSample->pdf)
       + bxdf->AmbientRadiance(inct));
}

PathTracer::PathTracer(uint32_t minDepth, Real stopProb)
    : minDepth_(minDepth), stopProb_(stopProb), stopCoef_(static_cast<float>(1 / stopProb))
{
    AGZ_ASSERT(minDepth >= 1);
}

Spectrum PathTracer::GetRadiance(const Scene &scene, const Ray &r) const
{
    return Trace(scene, r, 1);
}

AGZ_NS_END(Atrc)
