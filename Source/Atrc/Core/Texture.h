#pragma once

#include <Atrc/Core/Common.h>

AGZ_NS_BEG(Atrc)

class Texture
{
public:

    virtual ~Texture() = default;

    virtual Spectrum Sample(const Vec2 &uv) const = 0;
};

AGZ_NS_END(Atrc)
