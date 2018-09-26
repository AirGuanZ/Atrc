#pragma once

#include "../Common.h"
#include "../Math/Ray.h"
#include "../Misc/Spectrum.h"

AGZ_NS_BEG(Atrc)

ATRC_INTERFACE Integrator
{
public:

    virtual ~Integrator() = default;

    virtual Spectrum GetRadiance(const Ray &r) const = 0;
};

AGZ_NS_END(Atrc)
