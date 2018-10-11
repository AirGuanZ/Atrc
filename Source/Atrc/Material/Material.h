#pragma once

#include <Atrc/Common.h>
#include <Atrc/Material/BxDF.h>

AGZ_NS_BEG(Atrc)

ATRC_INTERFACE Material
{
public:

    virtual ~Material() = default;

    virtual Box<BxDF> GetBxDF(const Intersection &inct, const Vec2r &matParam) const = 0;
};

AGZ_NS_END(Atrc)
