#pragma once

#include <Atrc/Common.h>
#include <Atrc/Material/BxDF.h>
#include <Atrc/Material/Material.h>

AGZ_NS_BEG(Atrc)

class NormalMaterial
    : ATRC_IMPLEMENTS Material,
      ATRC_PROPERTY AGZ::Uncopiable
{
public:

    Box<BxDF> GetBxDF(const Intersection &inct, const Vec2r &matParam) const override;
};

AGZ_NS_END(Atrc)
