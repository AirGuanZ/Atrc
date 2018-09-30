#pragma once

#include <Atrc/Common.h>
#include <Atrc/Entity/Entity.h>
#include <Atrc/Math/Math.h>

AGZ_NS_BEG(Atrc)

class DiffuseSphere
    : ATRC_IMPLEMENTS Entity,
      ATRC_PROPERTY AGZ::Uncopiable
{
    Real radius_;
    Spectrum diffuseColor_;

    Transform local2World_;

public:

    DiffuseSphere(Real radius, const Spectrum &color, const Transform &local2World);

    bool HasIntersection(const Ray &r) const override;

    bool EvalIntersection(const Ray &r, Intersection *inct) const override;

    RC<BxDF> GetBxDF(const Intersection &inct) const override;
};

AGZ_NS_END(Atrc)
