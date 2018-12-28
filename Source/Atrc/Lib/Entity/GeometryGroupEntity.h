#pragma once

#include <Atrc/Lib/Core/Entity.h>
#include <Atrc/Lib/Core/Transform.h>

namespace Atrc
{

class GeometryGroupEntity : public Entity
{
    const Geometry **geometry_;
    const Material **material_;
    const MediumInterface *mediumInterface_;
    int count_;

    const Transform local2World_;
    AABB worldBound_;

public:

    GeometryGroupEntity(
        const Geometry **geometry,
        const Material **material,
        const MediumInterface *mediumInterface,
        int count, const Transform local2World) noexcept;

    bool HasIntersection(const Ray &_r) const noexcept override;

    bool FindIntersection(const Ray &_r, Intersection *inct) const noexcept override;

    AABB GetWorldBound() const noexcept override;

    const Light *AsLight() const noexcept override;

    Light *AsLight() noexcept override;
};

} // namespace Atrc
