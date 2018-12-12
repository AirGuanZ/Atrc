#include <Atrc/Lib/Entity/GeometricEntity.h>

namespace Atrc
{

GeometricEntity::GeometricEntity(const Geometry *geometry, const Material *material) noexcept
    : geometry_(geometry), material_(material)
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
    return true;
}

AABB GeometricEntity::GetWorldBound() const noexcept
{
    AGZ_ASSERT(geometry_);
    return geometry_->GetWorldBound();
}

CoordSystem GeometricEntity::GetShadingCoordSys(const Intersection &inct) const noexcept
{
    AGZ_ASSERT(inct.entity == this);
    return geometry_->GetShadingCoordSys(inct);
}

Vec2 GeometricEntity::GetShadingUV(const Intersection &inct) const noexcept
{
    AGZ_ASSERT(inct.entity == this);
    return geometry_->GetShadingUV(inct);
}

const Material *GeometricEntity::GetMaterial([[maybe_unused]] const Intersection &inct) const noexcept
{
    AGZ_ASSERT(material_ && inct.entity == this);
    return material_;
}

const Light *GeometricEntity::AsLight() const noexcept
{
    return nullptr;
}

} // namespace Atrc
