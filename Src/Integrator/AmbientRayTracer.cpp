#include "AmbientRayTracer.h"

AGZ_NS_BEG(Atrc)

Spectrum AmbientRayTracer::GetRadiance(const Scene &scene, const Ray &r) const
{
    Option<EntityIntersection> inct = EvalIntersection(scene, r);
    if(!inct)
        return SPECTRUM::BLACK;

    auto ret = inct->bxdf->AmbientRadiance();
    delete inct->bxdf;
    return ret;
}

AGZ_NS_END(Atrc)
