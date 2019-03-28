#pragma once

#include <Atrc/Core/Light/InfiniteLight.h>

namespace Atrc
{

class EnvironmentLight : public InfiniteLight
{
    const Texture *tex_;

public:

    EnvironmentLight(const Texture *tex, const Transform &local2World) noexcept;

    Spectrum NonAreaLe(const Ray &r) const noexcept override;
};

} // namespace Atrc
