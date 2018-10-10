#pragma once

#include <Atrc/Common.h>
#include <Atrc/Entity/Entity.h>

AGZ_NS_BEG(Atrc)

class Cube
    : ATRC_IMPLEMENTS Entity
{
    Real halfSideLen_;

public:

    explicit Cube(Real halfSideLen);

    bool HasIntersection(const Ray &r) const override;

    bool EvalIntersection(const Ray &r, Intersection *inct) const override;

    AABB GetBoundingBox() const override;

    Real SurfaceArea() const override;
};

AGZ_NS_END(Atrc)
