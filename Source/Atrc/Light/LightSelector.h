#pragma once

#include <Atrc/Common.h>
#include <Atrc/Light/Light.h>

AGZ_NS_BEG(Atrc)

struct LightSelectorSample
{
    const Light *light;
    Real pdf;
};

class LightSelector
{
    std::vector<const Light*> lights_;

public:

    explicit LightSelector(const std::vector<const Light*> &&lights);

    // Sample a light source to compute direct illumination at inct
    // ret.light == nullptr means there is no available light source
    LightSelectorSample Sample(const Intersection &inct) const;

    Real PDF(const Light *light) const;
};

AGZ_NS_END(Atrc)
