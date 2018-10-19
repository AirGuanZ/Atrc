#pragma once

#include <Atrc/Core/Common.h>
#include <Atrc/Core/Entity.h>
#include <Atrc/Core/Geometry.h>

AGZ_NS_BEG(Atrc)

class GeometryEntity : public Entity
{
    const Geometry *geometry_;
    const Material *material_;

public:

    GeometryEntity(const Geometry *geometry, const Material *material);

    bool HasIntersection(const Ray &r) const override;

    bool FindIntersection(const Ray &r, SurfacePoint *sp) const override;

    AABB WorldBound() const override;

    const Material *GetMaterial(const SurfacePoint &sp) const override;

    const Light *AsAreaLight() const override;
};

AGZ_NS_END(Atrc)
