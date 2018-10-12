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

    explicit LightManager(std::vector<Light*> &&lights);

    LightManagerSample Sample(const Intersection &inct) const;
};

AGZ_NS_END(Atrc)
