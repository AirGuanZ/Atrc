#include <Atrc/Lib/Entity/GeometricEntity.h>

namespace Atrc
{

GeometricEntity::GeometricEntity(const Geometry *geometry, const Material *material, const MediumInterface &mediumInterface) noexcept
    : geometry_(geometry), material_(material), mediumInterface_(mediumInterface)
{
    AGZ_ASSERT(geometry && material);
}

bool GeometricEntity::HasIntersection(const Ray &r) const noexcept
{
    AGZ_ASSERT(geometry_);
    return geometry_->HasIntersection(r);
}

bool GeometricEntity::FindIntersection(const Ray &r, Intersection *inct) const noexcept
{
    AGZ_ASSERT(geometry_ && inct);
    if(!geometry_->FindIntersection(r, inct))
        return false;
    inct->entity = this;
    inct->material = material_;
    inct->mediumInterface = mediumInterface_;
    return true;
}

AABB GeometricEntity::GetWorldBound() const noexcept
{
    AGZ_ASSERT(geometry_);
    return geometry_->GetWorldBound();
}

const Light *GeometricEntity::AsLight() const noexcept
{
    return nullptr;
}

Light *GeometricEntity::AsLight() noexcept
{
    return nullptr;
}

} // namespace Atrc
