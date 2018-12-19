#pragma once

#include <Atrc/Lib/Core/Common.h>
#include <Atrc/Lib/Core/Ray.h>
#include <Atrc/Lib/Core/SurfacePoint.h>

namespace Atrc
{

class Scene;

class Light
{
public:

    struct SampleWiResult
    {
        Vec3 pos;
        Vec3 wi;
        Spectrum radiance;
        Real pdf;
        bool isDelta;
    };

    virtual ~Light() = default;

    virtual void PreprocessScene(const Scene &scene) = 0;

    virtual SampleWiResult SampleWi(const Intersection &inct, const ShadingPoint &shd, const Vec3 &sample) const noexcept = 0;

    virtual Real SampleWiAreaPDF(const Vec3 &pos, const Vec3 &nor, const Intersection &inct, const ShadingPoint &shd) const noexcept = 0;

    virtual Real SampleWiNonAreaPDF(const Vec3 &wi, const Intersection &inct, const ShadingPoint &shd) const noexcept = 0;

    virtual Spectrum AreaLe(const Intersection &inct) const noexcept = 0;

    virtual Spectrum NonAreaLe(const Ray &r) const noexcept = 0;
};

} // namespace Atrc
