#pragma once

#include <Atrc/Common.h>
#include <Atrc/Material/BxDF.h>
#include <Atrc/Material/Material.h>

AGZ_NS_BEG(Atrc)

class MetalMaterial
    : ATRC_IMPLEMENTS Material,
      ATRC_PROPERTY AGZ::Uncopiable
{
    Spectrum color_;
    Real roughness_;

public:

    MetalMaterial(const Spectrum &color, Real roughness);

    Box<BxDF> GetBxDF(const Intersection &inct, const Vec2r &matParam) const override;
};

AGZ_NS_END(Atrc)
