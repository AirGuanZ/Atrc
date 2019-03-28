#pragma once

#include <Atrc/Core/Core/AABB.h>
#include <Atrc/Core/Core/Light.h>
#include <Atrc/Core/Core/Material.h>

namespace Atrc
{

class Entity
{
public:

    virtual ~Entity() = default;

    virtual bool HasIntersection(const Ray &r) const noexcept = 0;

    virtual bool FindIntersection(const Ray &r, Intersection *inct) const noexcept = 0;

    virtual AABB GetWorldBound() const = 0;

    virtual const Light *AsLight() const noexcept = 0;

    virtual Light *AsLight() noexcept = 0;
};

} // namespace Atrc
