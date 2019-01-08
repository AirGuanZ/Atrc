#pragma once

#include <Utils/Texture.h>

#include <Atrc/Lib/Core/Common.h>

namespace Atrc
{

class EnvMapSampler
{
    AGZ::Texture2D<Real> distrib_;

public:

    EnvMapSampler(const AGZ::Texture2D<Color3f> &tex, uint32_t distribWidth, uint32_t distribHeight);

    struct SampleResult
    {
        Vec3 dir;
        Spectrum coef;
        Real pdf;
    };

    SampleResult Sample(const Vec3 &sample) const noexcept;
};

} // namespace Atrc
