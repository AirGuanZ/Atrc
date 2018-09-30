#pragma once

#include <Atrc/Common.h>
#include <Atrc/Entity/Sphere.h>
#include <Atrc/Math/Math.h>

AGZ_NS_BEG(Atrc)

class GlassSphere
    : ATRC_PROPERTY AGZ::Uncopiable,
      public Sphere
{
    Spectrum reflectedColor_, refractedColor_;
    Real refIdx_;

public:

    GlassSphere(
        Real radius, const Transform &local2World,
        const Spectrum &reflColor, const Spectrum &refrColor, Real refIdx);

    RC<BxDF> GetBxDF(const Intersection &inct) const override;
};

AGZ_NS_END(Atrc)
