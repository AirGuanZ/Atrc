#pragma once

#include <Atrc/Lib/Core/Entity.h>
#include <Atrc/Lib/Core/Geometry.h>
#include <Atrc/Lib/Core/Material.h>
#include <Atrc/Lib/Core/Medium.h>

namespace Atrc
{

class GeometricEntity : public Entity
{
    const Geometry *geometry_;
    const Material *material_;
    const MediumInterface mediumInterface_;

public:

    GeometricEntity(const Geometry *geometry, const Material *material, const MediumInterface &mediumInterface) noexcept;

    bool HasIntersection(const Ray &r) const noexcept override;

    bool FindIntersection(const Ray &r, Intersection *inct) const noexcept override;

    AABB GetWorldBound() const noexcept override;

    const Material *GetMaterial(const Intersection &inct) const noexcept override;

    const Light *AsLight() const noexcept override;

    Light *AsLight() noexcept override;
};

} // namespace Atrc
