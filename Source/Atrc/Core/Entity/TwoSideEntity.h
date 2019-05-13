#pragma once

#include <Atrc/Core/Core/Entity.h>

namespace Atrc
{
    
class TwoSideEntity : public Entity
{
    Entity *internal_;

public:

    explicit TwoSideEntity(Entity *internalEntity) noexcept;

    bool HasIntersection(const Ray &r) const noexcept override;

    bool FindIntersection(const Ray &r, Intersection *inct) const noexcept override;

    AABB GetWorldBound() const override;

    const Light *AsLight() const noexcept override;

    Light *AsLight() noexcept override;
};

} // namespace Atrc
