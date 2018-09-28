#include "Integrator.h"

AGZ_NS_BEG(Atrc)

bool Integrator::HasIntersection(const Scene& scene, const Ray& r) const
{
    for(auto ent : scene.GetEntities())
    {
        if(ent->HasIntersection(r))
            return true;
    }
    return false;
}

Option<EntityIntersection> Integrator::EvalIntersection(const Scene &scene, const Ray &r) const
{
    Option<EntityIntersection> ret = None;
    for(auto ent : scene.GetEntities())
    {
        auto inct = ent->EvalIntersection(r);
        if(inct && (!ret || inct->geoInct.t < ret->geoInct.t))
            ret = inct;
    }
    return ret;
}

AGZ_NS_END(Atrc)
