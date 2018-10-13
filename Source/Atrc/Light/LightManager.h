#pragma once

#include <Atrc/Common.h>
#include <Atrc/Light/Light.h>

AGZ_NS_BEG(Atrc)

struct LightManagerSample
{
    const Light *light;
    Real pdf;
};

class LightManager
{
public:

    explicit LightManager(std::vector<Light*> &&lights)
    {
        
    }

    // Sample a light source to compute direct illumination at inct
    // ret.light == nullptr means there is no available light source
    LightManagerSample Sample(const Intersection &inct) const
    {
        return { nullptr, 0.0 };
    }
};

AGZ_NS_END(Atrc)
