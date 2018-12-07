#include <Atrc/Material/NormalMapper.h>

AGZ_NS_BEG(Atrc)

NormalMapper::NormalMapper(const Texture *tex)
    : tex_(tex)
{
    AGZ_ASSERT(tex);
}

Vec3 NormalMapper::GetLocalNormal(const Vec2 &texCoord) const
{
    auto texel = tex_->Sample(texCoord);
    return texel.Map([](float c)
    {
        return 2 * Real(c) - 1;
    });
}

AGZ_NS_END(Atrc)
