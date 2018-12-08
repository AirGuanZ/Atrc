#pragma once

#include <Atrc/Core/Core.h>

AGZ_NS_BEG(Atrc)

class NormalMapper
{
public:

    virtual ~NormalMapper() = default;

    virtual Vec3 GetLocalNormal(const Vec2 &texCoord) const = 0;
};

class TrivialNormalMapper : public NormalMapper
{
public:

    Vec3 GetLocalNormal([[maybe_unused]] const Vec2 &texCoord) const override
    {
        return Vec3::UNIT_Z();
    }
};

class TextureNormalMapper : public NormalMapper
{
    const Texture *tex_;

public:

    explicit TextureNormalMapper(const Texture *tex)
        : tex_(tex)
    {
        AGZ_ASSERT(tex);
    }

    Vec3 GetLocalNormal(const Vec2 &texCoord) const override
    {
        auto texel = tex_->Sample(texCoord);
        return texel.Map([](float c) { return 2 * Clamp<Real>(c, 0, 1) - 1; }).Normalize();
    }
};

AGZ_NS_END(Atrc)
