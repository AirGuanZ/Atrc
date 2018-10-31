#pragma once

#include <Atrc/Core/Core.h>

AGZ_NS_BEG(Atrc)

class GeometricEntity : public Entity
{
    const Geometry *geometry_;
    const Material *material_;

public:

    GeometricEntity(const Geometry *geometry, const Material *material);

    bool HasIntersection(const Ray &r) const override;

    bool FindIntersection(const Ray &r, SurfacePoint *sp) const override;

    AABB WorldBound() const override;

    const Material *GetMaterial(const SurfacePoint &sp) const override;
};

AGZ_NS_END(Atrc)
