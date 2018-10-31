#include <Atrc/Core/Entity.h>

AGZ_NS_BEG(Atrc)

bool Entity::HasIntersection(const Ray &r) const
{
    SurfacePoint sp;
    return FindIntersection(r, &sp);
}

const Light *Entity::AsLight() const
{
    return nullptr;
}

Light *Entity::AsLight()
{
    return nullptr;
}

AGZ_NS_END(Atrc)
