#include <Atrc/Lib/Core/Geometry.h>
#include <Atrc/Lib/Entity/GeometryGroupEntity.h>

namespace Atrc
{

GeometryGroupEntity::GeometryGroupEntity(
        const Geometry **geometry,
        const Material **material,
        const MediumInterface *mediumInterface,
        int count, const Transform local2World) noexcept
    : geometry_(geometry), material_(material), mediumInterface_(mediumInterface), count_(count),
      local2World_(local2World)
{

}

bool GeometryGroupEntity::HasIntersection(const Ray &_r) const noexcept
{
    Ray r = local2World_.ApplyInverseToRay(_r);
    for(int i = 0; i < count_; ++i)
    {
        if(geometry_[i]->HasIntersection(r))
            return true;
    }
    return false;
}

bool GeometryGroupEntity::FindIntersection(const Ray &_r, Intersection *inct) const noexcept
{
    Ray r = local2World_.ApplyInverseToRay(_r);
    
    GeometryIntersection tInct, bestInct;
    bestInct.t = RealT::Infinity();
    int bestInctGeoIndex = -1;
    for(int i = 0; i < count_; ++i)
    {
        if(geometry_[i]->FindIntersection(r, &tInct) && tInct.t < bestInct.t)
        {
            bestInct = tInct;
            bestInctGeoIndex = i;
        }
    }

    if(bestInctGeoIndex < 0)
        return false;
    
    inct->t               = bestInct.t;
    inct->pos             = local2World_.ApplyToPoint(bestInct.pos);
    inct->wr              = -_r.d;
    inct->uv              = bestInct.uv;
    inct->coordSys        = local2World_.ApplyToCoordSystem(bestInct.coordSys);
    inct->usr.uv          = bestInct.usr.uv;
    inct->usr.coordSys    = local2World_.ApplyToCoordSystem(bestInct.usr.coordSys);
    inct->entity          = this;
    inct->material        = material_[bestInctGeoIndex];
    inct->mediumInterface = mediumInterface_[bestInctGeoIndex];

    return true;
}

AABB GeometryGroupEntity::GetWorldBound() const noexcept
{
    return worldBound_;
}

const Light *GeometryGroupEntity::AsLight() const noexcept
{
    return nullptr;
}

Light *GeometryGroupEntity::AsLight() noexcept
{
    return nullptr;
}

} // namespace Atrc
