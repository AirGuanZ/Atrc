#pragma once

#include <Atrc/Lib/Core/Texture.h>

namespace Atrc
{

class TextureMultiplier : public Texture
{
    const Texture *lhs_;
    const Texture *rhs_;

public:

    TextureMultiplier(const Texture *lhs, const Texture *rhs) noexcept;

    Spectrum Sample(const Vec2 &uv) const noexcept override;
};

} // namespace Atrc
