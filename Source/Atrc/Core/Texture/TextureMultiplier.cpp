#include <Atrc/Core/Texture/TextureMultiplier.h>

namespace Atrc
{

TextureMultiplier::TextureMultiplier(const Texture *lhs, const Texture *rhs) noexcept
    : lhs_(lhs), rhs_(rhs)
{

}

Spectrum TextureMultiplier::Sample(const Vec2 &uv) const noexcept
{
    auto l = lhs_->Sample(uv);
    auto r = rhs_->Sample(uv);
    return l * r;
}

} // namespace Atrc
