#include <Atrc/Lib/Texture/ConstantTexture.h>

AGZ_NS_BEG(Atrc)

ConstantTexture::ConstantTexture(const Spectrum &texel) noexcept
    : texel_(texel)
{

}

Spectrum ConstantTexture::Sample([[maybe_unused]] const Vec2 &uv) const noexcept
{
    return texel_;
}

AGZ_NS_END(Atrc)
