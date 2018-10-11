#pragma once

#include <Atrc/Common.h>
#include <Atrc/Material/BxDF.h>
#include <Atrc/Material/Material.h>

AGZ_NS_BEG(Atrc)

class AmbientMaterial
    : ATRC_IMPLEMENTS Material,
      ATRC_PROPERTY AGZ::Uncopiable
{
    Spectrum color_;

public:

    explicit AmbientMaterial(const Spectrum &color);

    Box<BxDF> GetBxDF(const Intersection &inct, const Vec2r &matParam) const override;
};

AGZ_NS_END(Atrc)
