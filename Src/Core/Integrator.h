#pragma once

#include "../Common.h"
#include "../Core/Spectrum.h"
#include "../Math/Ray.h"

AGZ_NS_BEG(Atrc)

ATRC_INTERFACE Integrator
{
public:

    virtual ~Integrator() = default;

    virtual Spectrum GetRadiance(const Ray &r) const = 0;
};

AGZ_NS_END(Atrc)
