#pragma once

#include <Atrc/Common.h>
#include <Atrc/Math/Math.h>
#include "Atrc/Light/LightManager.h"

AGZ_NS_BEG(Atrc)

struct Scene
{
    const Camera *camera         = nullptr;
    const LightManager *lightMgr = nullptr;
    std::vector<const Entity*> entities;
};

ATRC_INTERFACE Integrator
{
protected:

    static bool FindClosestIntersection(const Scene &scene, const Ray &r, Intersection *inct);

    static bool HasIntersection(const Scene &scene, const Ray &r);

public:

    virtual ~Integrator() = default;

    virtual Spectrum GetRadiance(const Scene &scene, const Ray &r) const = 0;
};

AGZ_NS_END(Atrc)
