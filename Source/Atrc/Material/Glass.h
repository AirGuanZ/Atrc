#pragma once

#include <Atrc/Common.h>
#include <Atrc/Material/BxDF.h>
#include <Atrc/Material/Material.h>

AGZ_NS_BEG(Atrc)

class GlassMaterial
    : ATRC_IMPLEMENTS Material,
      ATRC_PROPERTY AGZ::Uncopiable
{
    Spectrum reflColor_, refrColor_;
    Real refIdx_;

public:

    explicit GlassMaterial(const Spectrum &reflRefrColor, Real refIdx = 1.5)
        : GlassMaterial(reflRefrColor, reflRefrColor, refIdx)
    {
        
    }

    GlassMaterial(const Spectrum &reflColor, const Spectrum &refrColor, Real refIdx = 1.5);

    Box<BxDF> GetBxDF(const Intersection &inct, const Vec2r &matParam) const override;
};

AGZ_NS_END(Atrc)
