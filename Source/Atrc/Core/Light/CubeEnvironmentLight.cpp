#include <AGZUtils/Utils/Texture.h>

#include <Atrc/Core/Core/Scene.h>
#include <Atrc/Core/Light/CubeEnvironmentLight.h>

namespace Atrc
{

CubeEnvironmentLight::CubeEnvironmentLight(const Texture **envTex, const Transform &local2World) noexcept
    : InfiniteLight(local2World)
{
    AGZ_ASSERT(envTex);
    for(int i = 0; i < 6; ++i)
    {
        AGZ_ASSERT(envTex[i]);
        envTex_[i] = envTex[i];
    }
}

Spectrum CubeEnvironmentLight::NonAreaLe(const Ray &r) const noexcept
{
    auto [face, uv] = AGZ::CubeMapper<Real>::Map(local2World_.ApplyInverseToVector(r.d).Normalize().xzy());
    return envTex_[int(face)]->Sample(uv);
}

} // namespace Atrc
