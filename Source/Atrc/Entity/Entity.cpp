#include <Atrc/Entity/Entity.h>

AGZ_NS_BEG(Atrc)

bool Entity::HasIntersection(const Ray &r) const
{
    return EvalIntersection(r).has_value();
}

Spectrum Entity::AmbientRadiance() const
{
    return SPECTRUM::BLACK;
}

AGZ_NS_END(Atrc)
