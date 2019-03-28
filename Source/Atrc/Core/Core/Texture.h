#pragma once

#include <Atrc/Core/Core/Common.h>

namespace Atrc
{

class Texture
{
public:

    virtual ~Texture() = default;

    virtual Spectrum Sample(const Vec2 &texCoord) const noexcept = 0;

    virtual Real Sample1(const Vec2 &texCoord) const noexcept { return Sample(texCoord).r; }
};

}
