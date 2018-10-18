#pragma once

#include <vector>

#include <Atrc/Core/Common.h>
#include <Atrc/Core/Entity.h>
#include <Atrc/Core/Light.h>
#include <Atrc/Core/SurfacePoint.h>

AGZ_NS_BEG(Atrc)

class Scene
{
public:

    std::vector<const Entity*> entities_;
    std::vector<const Light*> lights_;
    const Camera *camera;

    const Camera *GetCamera() const;

    bool HasIntersection(const Ray &r) const;

    bool FindCloestIntersection(const Ray &r, SurfacePoint *sp) const;
};

AGZ_NS_END(Atrc)
