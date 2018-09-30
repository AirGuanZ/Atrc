#include <Atrc/Entity/Entity.h>

AGZ_NS_BEG(Atrc)

bool Entity::HasIntersection(const Ray &r) const
{
    Intersection inct;
    return EvalIntersection(r, &inct);
}

AGZ_NS_END(Atrc)
