#include <Atrc/Entity/GeometricEntity.h>

AGZ_NS_BEG(Atrc)

GeometryEntity::GeometryEntity(const Geometry *geometry, const Material *material)
    : geometry_(geometry), material_(material)
{
    AGZ_ASSERT(geometry && material);
}

bool GeometryEntity::HasIntersection(const Ray &r) const
{
    AGZ_ASSERT(geometry_);
    return geometry_->HasIntersection(r);
}

bool GeometryEntity::FindIntersection(const Ray &r, SurfacePoint *sp) const
{
    AGZ_ASSERT(geometry_ && sp);
    if(!geometry_->FindIntersection(r, sp))
        return false;
    sp->entity = this;
    return true;
}

AABB GeometryEntity::WorldBound() const
{
    AGZ_ASSERT(geometry_);
    return geometry_->WorldBound();
}

const Material *GeometryEntity::GetMaterial(const SurfacePoint &sp) const
{
    AGZ_ASSERT(material_ && sp.entity == this);
    return material_;
}

const Light *GeometryEntity::AsLight() const
{
    return nullptr;
}

Light* GeometryEntity::AsLight()
{
    return nullptr;
}

AGZ_NS_END(Atrc)
