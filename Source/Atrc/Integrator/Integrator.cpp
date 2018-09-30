#include <Atrc/Entity/Entity.h>
#include <Atrc/Integrator/Integrator.h>

AGZ_NS_BEG(Atrc)

bool Integrator::FindClosestIntersection(const Scene &scene, const Ray &r, Intersection *inct)
{
    AGZ_ASSERT(inct);

    bool incted = false;
    for(auto ent : scene.entities)
    {
        Intersection newInct;
        if(ent->EvalIntersection(r, &newInct) &&
           (!incted || newInct.t < inct->t))
        {
            *inct = newInct;
            incted = true;
        }
    }

    return incted;
}

AGZ_NS_END(Atrc)
