#pragma once

#include <Atrc/Common.h>
#include <Atrc/Math/Math.h>

AGZ_NS_BEG(Atrc)

class Scene
{
public:

    const Camera *camera;
    std::vector<const Entity*> entities;
};

ATRC_INTERFACE Integrator
{
public:

    virtual ~Integrator() = default;

    virtual Spectrum GetRadiance(const Scene &scene, const Ray &r) const = 0;
};

AGZ_NS_END(Atrc)
