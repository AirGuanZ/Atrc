#pragma once

#include <Atrc/Core/Core.h>

AGZ_NS_BEG(Atrc)

class ConstantTexture : public Texture
{
    Spectrum texel_;

public:

    explicit ConstantTexture(const Spectrum &texel);

    Spectrum Sample(const Vec2 &uv) const override;
};

AGZ_NS_END(Atrc)
