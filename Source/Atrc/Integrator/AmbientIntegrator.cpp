#include <Atrc/Entity/Entity.h>
#include <Atrc/Material/BxDF.h>
#include <Atrc/Integrator/AmbientIntegrator.h>

AGZ_NS_BEG(Atrc)

Spectrum AmbientIntegrator::GetRadiance(const Scene &scene, const Ray &r) const
{
    Intersection inct;
    if(!FindClosestIntersection(scene, r, &inct))
        return SPECTRUM::BLACK;
    return inct.entity->GetBxDF(inct)->AmbientRadiance(inct);
}

AGZ_NS_END(Atrc)
