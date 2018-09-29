#pragma once

#include <Atrc/Common.h>
#include <Atrc/Entity/Entity.h>

AGZ_NS_BEG(Atrc)

class ColoredSky
    : ATRC_IMPLEMENTS Entity,
      ATRC_PROPERTY AGZ::Uncopiable
{
    Spectrum top_, bottom_;

public:

    ColoredSky(const Spectrum &top, const Spectrum &bottom);

    bool HasIntersection(const Ray &r) const override;

    Option<Intersection> EvalIntersection(const Ray &r) const override;

    RC<BxDF> GetBxDF(const Intersection &inct) const override;
};

AGZ_NS_END(Atrc)
