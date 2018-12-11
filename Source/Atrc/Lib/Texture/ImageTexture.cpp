#include <Atrc/Lib/Texture/ImageTexture.h>

AGZ_NS_BEG(Atrc)

namespace
{
    float Byte2Spectrum(uint32_t b) { return b / 255.0f; }

    Spectrum NearestSampleStrategy(const AGZ::Texture2D<Color3b> &tex, const Vec2 &uv)
    {
        return AGZ::NearestSampler::Sample(tex, uv,
            [](const Color3b &c)
            {
                return c.Map(Byte2Spectrum);
            });
    }

    Spectrum LinearSampleStrategy(const AGZ::Texture2D<Color3b> &tex, const Vec2 &uv)
    {
        return AGZ::LinearSampler::Sample(tex, uv,
            [](const Color3b &c)
            {
                return c.Map(Byte2Spectrum);
            });
    }

    Real ClampTexCoord(Real value)
    {
        return AGZ::Math::Clamp<Real>(value, 0, 1);
    }
}

ImageTexture::ImageTexture(
    const AGZ::Texture2D<Color3b> &tex,
    SampleStrategy sampleStrategy, WrapStrategy wrapStrategy) noexcept
    : tex_(tex)
{
    switch(sampleStrategy)
    {
    case Nearest:
        sampler_ = &NearestSampleStrategy;
        break;
    case Linear:
        sampler_ = &LinearSampleStrategy;
        break;
    default:
        AGZ::Unreachable();
    }

    switch(wrapStrategy)
    {
    case Clamp:
        texCoordWrapper_ = &ClampTexCoord;
        break;
    default:
        AGZ::Unreachable();
    }
}

Spectrum ImageTexture::Sample(const Vec2 &uv) const noexcept
{
    return sampler_(tex_, uv.Map(texCoordWrapper_));
}

AGZ_NS_END(Atrc)
