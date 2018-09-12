#pragma once

#include "../Math/Ray.h"
#include "../Misc/Spectrum.h"

AGZ_NS_BEG(Atrc)

class Integrator
{
public:

    virtual ~Integrator() = default;

    virtual Spectrum Render(const Ray &r) const = 0;
};

AGZ_NS_END(Atrc)
