#pragma once

#include <Atrc/Common.h>
#include <Atrc/Entity/GeometryTemplate/Sphere.h>
#include <Atrc/Math/Math.h>

AGZ_NS_BEG(Atrc)

class MetalSphere
    : ATRC_PROPERTY AGZ::Uncopiable,
      public Sphere
{
    Spectrum reflectedColor_;
    Real roughness_;

public:

    MetalSphere(Real radius, const Transform &local2World, const Spectrum &color, Real roughness = 0.0);

    RC<BxDF> GetBxDF(const Intersection &inct) const override;
};

AGZ_NS_END(Atrc)
