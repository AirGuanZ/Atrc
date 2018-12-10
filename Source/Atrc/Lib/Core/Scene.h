#pragma once

#include <vector>

#include <Atrc/Lib/Core/AABB.h>
#include <Atrc/Lib/Core/Camera.h>
#include <Atrc/Lib/Core/Entity.h>
#include <Atrc/Lib/Core/Light.h>
#include <Atrc/Lib/Core/Ray.h>
#include <Atrc/Lib/Core/SurfacePoint.h>

namespace Atrc
{

class Scene
{
    std::vector<const Entity*> entities_;
    std::vector<const Light*> lights_;

    const Camera *camera_;

    AABB worldBound_;

public:

    Scene(
        const Entity **entities, size_t entityCount,
        const Light **lights,    size_t lightCount,
        const Camera *camera);
    
    const Camera *GetCamera() const noexcept;

    const std::vector<const Entity*> GetEntities() const noexcept;

    const std::vector<const Light*> GetLights() const noexcept;

    const AABB &GetWorldBound() const noexcept;

    struct SampleLightResult
    {
        const Light *light;
        Real pdf;
    };

    Option<SampleLightResult> SampleLight() const noexcept;

    Real SampleLightPDF(const Light *light) const noexcept;

    bool HasIntersection(const Ray &r) const noexcept;

    bool FindIntersection(const Ray &r, Intersection *inct) const noexcept;
};

} // namespace Atrc
