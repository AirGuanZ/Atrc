#pragma once

#include <Utils.h>

#include "../Math/Ray.h"
#include "../Misc/Spectrum.h"

AGZ_NS_BEG(Atrc)

class Renderer
{
public:

    virtual ~Renderer() = default;

    virtual Spectrum ComputeRadiance(const Ray &ray) const = 0;
};

AGZ_NS_END(Atrc)
