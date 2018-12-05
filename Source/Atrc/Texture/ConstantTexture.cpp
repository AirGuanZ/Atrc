#include <Atrc/Texture/ConstantTexture.h>

AGZ_NS_BEG(Atrc)

ConstantTexture::ConstantTexture(const Spectrum &texel)
    : texel_(texel)
{

}

Spectrum ConstantTexture::Sample([[maybe_unused]] const Vec2 &uv) const
{
    return texel_;
}

AGZ_NS_END(Atrc)
