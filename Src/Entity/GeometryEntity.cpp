#include "GeometryEntity.h"

AGZ_NS_BEG(Atrc)

GeometryEntity::GeometryEntity(
    const GeometryObject *geoObj, const Material *mat,
    const Transform &local2World)
    : geometry_(geoObj), material_(mat), local2World_(local2World)
{
    AGZ_ASSERT(geoObj && mat);
}

bool GeometryEntity::HasIntersection(const Ray &r) const
{
    return geometry_->HasIntersection(local2World_.ApplyInverseToRay(r));
}

Option<EntityIntersection> GeometryEntity::EvalIntersection(const Ray &r) const
{
    auto geoInct = geometry_->EvalIntersection(local2World_.ApplyInverseToRay(r));
    if(!geoInct)
        return None;
    auto local = local2World_.ApplyToSurfaceLocal(geoInct->local);
    return EntityIntersection{
        GeometryIntersection{
            geoInct->t, local
        },
        this,
        material_,
        RC<BxDF>(material_->GetBxDF(local))
    };
}

AGZ_NS_END(Atrc)
