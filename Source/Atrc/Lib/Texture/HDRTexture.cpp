#include <Utils/Misc.h>

#include <Atrc/Lib/Texture/HDRTexture.h>

AGZ_NS_BEG(Atrc)

namespace
{
    Spectrum NearestSampleStrategy(const AGZ::Texture2D<Color3f> &tex, const Vec2 &uv)
    {
        return AGZ::NearestSampler::Sample(tex, uv,
            [](const Color3f &c)
            {
                return c.Map(AGZ::TypeOpr::StaticCaster<Real, float>);
            });
    }

    Spectrum LinearSampleStrategy(const AGZ::Texture2D<Color3f> &tex, const Vec2 &uv)
    {
        return AGZ::LinearSampler::Sample(tex, uv,
            [](const Color3f &c)
            {
                return c.Map(AGZ::TypeOpr::StaticCaster<Real, float>);
            });
    }

    Real ClampTexCoord(Real value)
    {
        return AGZ::Math::Clamp<Real>(value, 0, 1);
    }
}

HDRTexture::HDRTexture(
    const AGZ::Texture2D<Color3f> &tex,
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

Spectrum HDRTexture::Sample(const Vec2 &uv) const noexcept
{
    return sampler_(tex_, (reverseV_ ? Vec2(uv.u, 1 - uv.v) : uv).Map(texCoordWrapper_));
}

AGZ_NS_END(Atrc)
