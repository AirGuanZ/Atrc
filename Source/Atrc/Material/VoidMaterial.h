#pragma once

#include <Atrc/Material/Material.h>

AGZ_NS_BEG(Atrc)

class VoidMaterial
    : ATRC_IMPLEMENTS Material
{
public:

    Box<BxDF> GetBxDF(const Intersection &inct, const Vec2r &matParam) const override;
};

AGZ_NS_END(Atrc)
