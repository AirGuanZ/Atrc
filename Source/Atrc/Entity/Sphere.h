#pragma once

#include <Atrc/Common.h>
#include <Atrc/Entity/Entity.h>
#include <Atrc/Math/Math.h>

AGZ_NS_BEG(Atrc)

class Sphere
    : ATRC_IMPLEMENTS Entity
{
protected:

    Real radius_;
    Transform local2World_;

public:

    Sphere(Real radius, const Transform &local2World);

    bool HasIntersection(const Ray &r) const override;

    bool EvalIntersection(const Ray &r, Intersection *inct) const override;
};

AGZ_NS_END(Atrc)
