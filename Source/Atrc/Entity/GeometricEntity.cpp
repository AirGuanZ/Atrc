#include <Atrc/Entity/GeometricEntity.h>

AGZ_NS_BEG(Atrc)

GeometricEntity::GeometricEntity(const Geometry *geometry, const Material *material, MediumInterface mediumInterface)
    : geometry_(geometry), material_(material), mediumInterface_(mediumInterface)
{
    AGZ_ASSERT(geometry && material);
}

bool GeometricEntity::HasIntersection(const Ray &r) const
{
    AGZ_ASSERT(geometry_);
    return geometry_->HasIntersection(r);
}

bool GeometricEntity::FindIntersection(const Ray &r, SurfacePoint *sp) const
{
    AGZ_ASSERT(geometry_ && sp);
    if(!geometry_->FindIntersection(r, sp))
        return false;
    sp->entity = this;
	sp->mediumInterface = mediumInterface_;
    return true;
}

AABB GeometricEntity::WorldBound() const
{
    AGZ_ASSERT(geometry_);
    return geometry_->WorldBound();
}

const Material *GeometricEntity::GetMaterial(const SurfacePoint &sp) const
{
    AGZ_ASSERT(material_ && sp.entity == this);
    return material_;
}

AGZ_NS_END(Atrc)
