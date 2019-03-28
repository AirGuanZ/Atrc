#include <Atrc/Core/Core/Texture.h>
#include <Atrc/Core/Light/EnvironmentLight.h>

namespace Atrc
{
    
EnvironmentLight::EnvironmentLight(const Texture *tex, const Transform &local2World) noexcept
    : InfiniteLight(local2World), tex_(tex)
{
    AGZ_ASSERT(tex);
}

Spectrum EnvironmentLight::NonAreaLe(const Ray &r) const noexcept
{
    Vec3 d = local2World_.ApplyInverseToVector(r.d).Normalize().xzy();
    Real u = Saturate(Phi(d) / (2 * PI));
    Real v = 1 - Saturate((Arcsin(Clamp<Real>(d.z, -1, 1)) + PI / 2) / PI);
    return tex_->Sample({ u, v });
}

} // namespace Atrc
