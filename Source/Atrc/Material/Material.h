#pragma once

#include <Atrc/Common.h>
#include <Atrc/Material/BxDF.h>

AGZ_NS_BEG(Atrc)

ATRC_INTERFACE Material
{
public:

    virtual ~Material() = default;

    virtual RC<BxDF> GetBxDF(const Intersection &inct) const = 0;
};

AGZ_NS_END(Atrc)
