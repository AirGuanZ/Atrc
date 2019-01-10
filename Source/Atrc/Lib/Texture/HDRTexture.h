#pragma once

#include <AGZUtils/Utils/Texture.h>

#include <Atrc/Lib/Core/Texture.h>

AGZ_NS_BEG(Atrc)

class HDRTexture : public Texture
{
    using TexCoordWrapper = Real(*)(Real);
    using Sampler = Spectrum(*)(const AGZ::Texture2D<Color3f>&, const Vec2&);

    const AGZ::Texture2D<Color3f> &tex_;

    TexCoordWrapper texCoordWrapper_;
    Sampler sampler_;

    bool reverseV_;

public:

    enum SampleStrategy
    {
        Nearest,
        Linear,
    };

    enum WrapStrategy
    {
        Clamp,
    };

    HDRTexture(
        const AGZ::Texture2D<Color3f> &tex,
        SampleStrategy sampleStrategy, WrapStrategy wrapStrategy,
        bool reverseV) noexcept;

    Spectrum Sample(const Vec2 &uv) const noexcept override;
};

AGZ_NS_END(Atrc)
