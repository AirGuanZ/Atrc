#pragma once

#include <Atrc/Common.h>
#include <Atrc/Light/Light.h>
#include <Atrc/Math/Math.h>

AGZ_NS_BEG(Atrc)

ATRC_INTERFACE Entity
{
public:

    virtual ~Entity() = default;

    virtual bool HasIntersection(const Ray &r) const;

    virtual bool EvalIntersection(const Ray &r, Intersection *inct) const = 0;

    virtual AABB GetBoundingBox() const = 0;

    virtual Real SurfaceArea() const = 0;

    virtual RC<BxDF> GetBxDF(const Intersection &inct) const = 0;

    virtual const Light *AsLight() const;
};

AGZ_NS_END(Atrc)
