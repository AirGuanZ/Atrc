#pragma once

#include <Atrc/Common.h>
#include <Atrc/Entity/GeometryTemplate/Sphere.h>
#include <Atrc/Math/Math.h>

AGZ_NS_BEG(Atrc)

class DiffuseSphere
    : ATRC_PROPERTY AGZ::Uncopiable,
      public Sphere
{
    Spectrum diffuseColor_;

public:

    DiffuseSphere(Real radius, const Transform &local2World, const Spectrum &color);

    RC<BxDF> GetBxDF(const Intersection &inct) const override;
};

AGZ_NS_END(Atrc)
