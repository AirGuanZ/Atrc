#pragma once

#include <Atrc/Core/Common.h>

#include <Atrc/Core/AABB.h>
#include <Atrc/Core/Light.h>
#include <Atrc/Core/Material.h>
#include <Atrc/Core/Ray.h>
#include <Atrc/Core/SurfacePoint.h>

AGZ_NS_BEG(Atrc)

class Entity
{
public:

    virtual ~Entity() = default;

    virtual bool HasIntersection(const Ray &r) const;

    virtual bool FindIntersection(const Ray &r, SurfacePoint *sp) const = 0;

    virtual AABB WorldBound() const = 0;

    virtual const Material *GetMaterial(const SurfacePoint &sp) const = 0;

    // 少数Entity自身可作为光源，否则返回nullptr
    virtual const Light *AsLight() const = 0;
};

AGZ_NS_END(Atrc)
