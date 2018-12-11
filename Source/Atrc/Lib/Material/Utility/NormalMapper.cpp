#include <Atrc/Lib/Material/Utility/NormalMapper.h>

namespace Atrc
{

Vec3 DefaultNormalMapper::GetLocalNormal([[maybe_unused]] const Vec2 &uv) const noexcept
{
    return Vec3::UNIT_Z();
}

TextureNormalMapper::TextureNormalMapper(const Texture *tex) noexcept
    : tex_(tex)
{
    AGZ_ASSERT(tex);
}

Vec3 TextureNormalMapper::GetLocalNormal(const Vec2 &uv) const noexcept
{
    return tex_->Sample(uv).Map([](float c) { return Real(2) * c - 1; });
}

} // namespace Atrc
