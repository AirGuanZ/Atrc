#pragma once

#include <Atrc/Lib/Core/Common.h>
#include <Atrc/Lib/Core/Ray.h>
#include <Atrc/Lib/Core/SurfacePoint.h>

namespace Atrc
{

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

    virtual SampleWiResult SampleWi(const Intersection &inct) const noexcept = 0;

    virtual Real SampleWiPDF(const Vec3 &pos, const Intersection &inct, bool onLight) const noexcept = 0;

    virtual Spectrum AreaLe(const Intersection &inct) const noexcept = 0;

    virtual Spectrum NonAreaLe(const Ray &r) const noexcept = 0;

    virtual Spectrum Power() const = 0;
};

} // namespace Atrc
