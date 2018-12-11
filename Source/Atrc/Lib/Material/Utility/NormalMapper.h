#pragma once

#include <Atrc/Lib/Core/Common.h>
#include <Atrc/Lib/Core/Texture.h>

namespace Atrc
{

class NormalMapper
{
public:

    virtual ~NormalMapper() = default;

    virtual Vec3 GetLocalNormal(const Vec2 &uv) const noexcept = 0;
};

class DefaultNormalMapper : public NormalMapper
{
public:

    Vec3 GetLocalNormal(const Vec2 &uv) const noexcept override;
};

class TextureNormalMapper : public NormalMapper
{
    const Texture *tex_;

public:

    explicit TextureNormalMapper(const Texture *tex) noexcept;

    Vec3 GetLocalNormal(const Vec2 &uv) const noexcept override;
};

} // namespace Atrc
