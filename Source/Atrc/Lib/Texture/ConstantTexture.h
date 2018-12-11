#pragma once

#include <Atrc/Lib/Core/Texture.h>

AGZ_NS_BEG(Atrc)

class ConstantTexture : public Texture
{
    Spectrum texel_;

public:

    explicit ConstantTexture(const Spectrum &texel) noexcept;

    Spectrum Sample(const Vec2 &uv) const noexcept override;
};

AGZ_NS_END(Atrc)
