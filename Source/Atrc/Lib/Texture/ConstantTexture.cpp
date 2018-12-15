#include <Atrc/Lib/Texture/ConstantTexture.h>

namespace Atrc
{

ConstantTexture::ConstantTexture(const Spectrum &texel) noexcept
    : texel_(texel)
{

}

Spectrum ConstantTexture::Sample([[maybe_unused]] const Vec2 &uv) const noexcept
{
    return texel_;
}

ConstantTexture1::ConstantTexture1(Real texel) noexcept
    : texel_(texel)
{

}

Spectrum ConstantTexture1::Sample([[maybe_unused]] const Vec2 &texCoord) const noexcept
{
    return Spectrum(texel_);
}

Real ConstantTexture1::Sample1([[maybe_unused]] const Vec2 &texCoord) const noexcept
{
    return texel_;
}

} // namespace Atrc
