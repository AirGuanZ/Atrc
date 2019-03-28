#include <Atrc/Core/Core/Light.h>

namespace Atrc
{
    
Light::SampleWiResult Light::SampleWi(const Intersection &inct, [[maybe_unused]] const ShadingPoint &shd, const Vec3 &sample) const noexcept
{
    return SampleWi(inct.pos, sample);
}

Real Light::SampleWiAreaPDF(const Vec3 &pos, const Vec3 &nor, const Intersection &inct, [[maybe_unused]] const ShadingPoint &shd) const noexcept
{
    return SampleWiAreaPDF(pos, nor, inct.pos);
}

Real Light::SampleWiNonAreaPDF(const Vec3 &wi, const Intersection &inct, [[maybe_unused]] const ShadingPoint &shd) const noexcept
{
    return SampleWiNonAreaPDF(wi, inct.pos);
}

} // namespace Atrc
