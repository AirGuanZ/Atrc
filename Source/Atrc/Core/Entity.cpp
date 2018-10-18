#include <Atrc/Core/Entity.h>

AGZ_NS_BEG(Atrc)

bool Entity::HasIntersection(const Ray &r) const
{
    SurfacePoint sp;
    return FindIntersection(r, &sp);
}

AGZ_NS_END(Atrc)
