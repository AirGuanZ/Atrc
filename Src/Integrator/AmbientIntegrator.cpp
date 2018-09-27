#include "AmbientIntegrator.h"

AGZ_NS_BEG(Atrc)

Spectrum AmbientIntegrator::GetRadiance(const Scene &scene, const Ray &r) const
{
    Option<EntityIntersection> inct;
    for(auto &entity : scene.GetEntities())
    {
        auto tInct = entity->EvalIntersection(r);

        if(tInct && (!inct || tInct->geometryIntersection.t <
                               inct->geometryIntersection.t))
        {
            if(inct)
                delete inct->bxdf;
            inct = tInct;
        }
    }

    if(!inct)
        return SPECTRUM::BLACK;

    auto ret = inct->bxdf->AmbientRadiance();
    delete inct->bxdf;
    return ret;
}

AGZ_NS_END(Atrc)
