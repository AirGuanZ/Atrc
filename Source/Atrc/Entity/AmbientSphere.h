#pragma once

#include <Atrc/Common.h>
#include <Atrc/Entity/Entity.h>
#include <Atrc/Math/Math.h>

AGZ_NS_BEG(Atrc)

class AmbientSphere
    : ATRC_IMPLEMENTS Entity,
      ATRC_PROPERTY AGZ::Uncopiable
{
    Real radius_;
    Spectrum ambientColor_;

    Transform local2World_;

public:

    AmbientSphere(Real radius, const Spectrum &color, const Transform &local2World);

    bool HasIntersection(const Ray &r) const override;

    bool EvalIntersection(const Ray &r, Intersection *inct) const override;

    RC<BxDF> GetBxDF(const Intersection &inct) const override;
};

AGZ_NS_END(Atrc)
