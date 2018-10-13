#include <Atrc/Entity/Entity.h>
#include <Atrc/Integrator/Integrator.h>

AGZ_NS_BEG(Atrc)

bool Integrator::FindClosestIntersection(const Scene &scene, const Ray &r, Intersection *inct)
{
    AGZ_ASSERT(inct);

    bool incted = false;
    Intersection newInct;
    for(auto ent : scene.entities)
    {
        if(ent->EvalIntersection(r, &newInct) &&
           (!incted || newInct.t < inct->t))
        {
            *inct = newInct;
            incted = true;
        }
    }

    return incted;
}

bool Integrator::HasIntersection(const Scene &scene, const Ray &r)
{
    for(auto ent : scene.entities)
    {
        if(ent->HasIntersection(r))
            return true;
    }
    return false;
}

AGZ_NS_END(Atrc)
