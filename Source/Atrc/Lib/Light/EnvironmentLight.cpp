#include <Atrc/Lib/Core/Texture.h>
#include <Atrc/Lib/Light/EnvironmentLight.h>

namespace Atrc
{
    
EnvironmentLight::EnvironmentLight(const Texture *tex, const Transform &local2World) noexcept
    : InfiniteLight(local2World), tex_(tex)
{
    AGZ_ASSERT(tex);
}

Spectrum EnvironmentLight::NonAreaLe(const Ray &r) const noexcept
{
    Vec3 d = local2World_.ApplyInverseToVector(r.d).Normalize();
    Real u = Saturate(Phi(d) / (2 * PI));
    Real v = Saturate(1 - Arcsin(Saturate(d.z)));
    return tex_->Sample({ u, v });
}

} // namespace Atrc
