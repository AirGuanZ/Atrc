#pragma once

#include "../Common.h"
#include "../Core/Entity.h"
#include "../Core/Material.h"
#include "../Math/Geometry.h"
#include "../Math/Transform.h"

AGZ_NS_BEG(Atrc)

class GeometryEntity : ATRC_IMPLEMENTS Entity, ATRC_PROPERTY AGZ::Uncopiable
{
    // No ownership
    const GeometryObject *geometry_;

    // No ownership
    const Material *material_;

    Transform local2World_;

public:

    GeometryEntity(
        const GeometryObject *geoObj, const Material *mat,
        const Transform &local2World);

    bool HasIntersection(const Ray &r) const override;

    Option<EntityIntersection> EvalIntersection(const Ray &r) const override;
};

AGZ_NS_END(Atrc)
