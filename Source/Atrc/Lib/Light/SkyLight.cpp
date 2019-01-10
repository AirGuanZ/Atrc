#include <Atrc/Lib/Core/Scene.h>
#include <Atrc/Lib/Light/SkyLight.h>

namespace Atrc
{

SkyLight::SkyLight(const Spectrum &topAndBottom, const Transform &local2World)
    : SkyLight(topAndBottom, topAndBottom, local2World)
{

}

SkyLight::SkyLight(const Spectrum &top, const Spectrum &bottom, const Transform &local2World)
    : InfiniteLight(local2World), top_(top), bottom_(bottom)
{

}

Spectrum SkyLight::NonAreaLe(const Ray &r) const noexcept
{
    Real topWeight = Real(0.5) * local2World_.ApplyInverseToVector(r.d).Normalize().z + Real(0.5);
    return topWeight * top_ + (1 - topWeight) * bottom_;
}

} // namespace Atrc
