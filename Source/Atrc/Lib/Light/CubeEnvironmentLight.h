#pragma once

#include <Atrc/Lib/Core/Texture.h>
#include <Atrc/Lib/Light/InfiniteLight.h>

namespace Atrc
{

class CubeEnvironmentLight : public InfiniteLight
{
    // 0 : +X
    // 1 : +Y
    // 2 : +Z
    // 3 : -X
    // 4 : -Y
    // 5 : -Z
    const Texture *envTex_[6];

public:

    explicit CubeEnvironmentLight(const Texture **envTex, const Transform &local2World) noexcept;

    Spectrum NonAreaLe(const Ray &r) const noexcept override;
};

} // namespace Atrc
