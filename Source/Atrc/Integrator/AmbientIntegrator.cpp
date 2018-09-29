#include <Atrc/Entity/Entity.h>
#include <Atrc/Material/BxDF.h>
#include <Atrc/Integrator/AmbientIntegrator.h>

AGZ_NS_BEG(Atrc)

Spectrum AmbientIntegrator::GetRadiance(const Scene &scene, const Ray &r) const
{
    Option<Intersection> inct = None;
    for(const Entity *ent : scene.entities)
        UpdateCloserIntersection(inct, ent->EvalIntersection(r));

    if(!inct)
        return SPECTRUM::BLACK;

    const Intersection &tInct = *inct;
    return tInct.entity->GetBxDF(tInct)->AmbientRadiance(tInct);
}

AGZ_NS_END(Atrc)
