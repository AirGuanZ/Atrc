#pragma once

#include <Atrc/Core/Core/Texture.h>

namespace Atrc
{

class ConstantTexture : public Texture
{
    Spectrum texel_;

public:

    explicit ConstantTexture(const Spectrum &texel) noexcept;

    Spectrum Sample(const Vec2 &uv) const noexcept override;
};

class ConstantTexture1 : public Texture
{
    Real texel_;

public:

    explicit ConstantTexture1(Real texel) noexcept;

    Spectrum Sample(const Vec2 &texCoord) const noexcept override;

    Real Sample1(const Vec2 &texCoord) const noexcept override;
};

} // namespace Atrc
