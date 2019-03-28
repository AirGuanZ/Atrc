#include <Atrc/Core/Texture/ImageTexture.h>

namespace Atrc
{

namespace
{
    Real Byte2Spectrum(uint32_t b) { return b / Real(255); }

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
    SampleStrategy sampleStrategy, WrapStrategy wrapStrategy,
    bool reverseV) noexcept
    : tex_(tex), reverseV_(reverseV)
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
    return sampler_(tex_, (reverseV_ ? Vec2(uv.u, 1 - uv.v) : uv).Map(texCoordWrapper_));
}

} // namespace Atrc
