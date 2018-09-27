#pragma once

#include <vector>

#include "../Common.h"
#include "../Core/Spectrum.h"
#include "../Math/Ray.h"
#include "Camera.h"
#include "Entity.h"

AGZ_NS_BEG(Atrc)

ATRC_INTERFACE Scene
{
public:

    virtual ~Scene() = default;

    virtual const Camera *GetCamera() const = 0;
    virtual const std::vector<const Entity*> &GetEntities() const = 0;
    virtual const std::vector<const Entity*> &GetLights() const = 0;
};

class SceneView
    : ATRC_IMPLEMENTS Scene
{
public:

    Camera *camera;
    std::vector<const Entity*> entities;
    std::vector<const Entity*> lights;

    const Camera *GetCamera() const override { return camera; }
    const std::vector<const Entity*> &GetEntities() const override { return entities; }
    const std::vector<const Entity*> &GetLights() const override { return lights; }
};

ATRC_INTERFACE Integrator
{
public:

    virtual ~Integrator() = default;

    virtual Spectrum GetRadiance(const Scene &scene, const Ray &r) const = 0;
};

AGZ_NS_END(Atrc)
