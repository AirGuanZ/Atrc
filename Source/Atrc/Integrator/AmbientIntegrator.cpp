#include <Atrc/Entity/Entity.h>
#include <Atrc/Material/BxDF.h>
#include <Atrc/Integrator/AmbientIntegrator.h>

AGZ_NS_BEG(Atrc)

Spectrum AmbientIntegrator::GetRadiance(const Scene &scene, const Ray &r) const
{
    Intersection inct; bool incted = false;
    for(const Entity *ent : scene.entities)
    {
        Intersection newInct;
        if(ent->EvalIntersection(r, &newInct))
        {
            if(!incted || newInct.t < inct.t)
                inct = newInct;
            incted = true;
        }
    }

    if(!incted)
        return SPECTRUM::BLACK;
    return inct.entity->GetBxDF(inct)->AmbientRadiance(inct);
}

AGZ_NS_END(Atrc)
