#pragma once

#include <Atrc/Core/Light/InfiniteLight.h>

namespace Atrc
{

class SkyLight : public InfiniteLight
{
    Spectrum top_;
    Spectrum bottom_;

public:

    explicit SkyLight(const Spectrum &topAndBottom, const Transform &local2World);

    SkyLight(const Spectrum &top, const Spectrum &bottom, const Transform &local2World);

    Spectrum NonAreaLe(const Ray &r) const noexcept override;
};

} // namespace Atrc
