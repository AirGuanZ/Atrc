#include <Atrc/Core/Geometry.h>

AGZ_NS_BEG(Atrc)

bool Geometry::HasIntersection(const Ray &r) const
{
    SurfacePoint sp;
    return FindIntersection(r, &sp);
}

AABB Geometry::WorldBound() const
{
    return local2World_.ApplyToAABB(LocalBound());
}

AGZ_NS_END(Atrc)
