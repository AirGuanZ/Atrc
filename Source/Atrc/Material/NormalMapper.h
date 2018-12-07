#pragma once

#include <Atrc/Core/Core.h>

AGZ_NS_BEG(Atrc)

class NormalMapper
{
    const Texture *tex_;

public:

    explicit NormalMapper(const Texture *tex);

    Vec3 GetLocalNormal(const Vec2 &texCoord) const;
};

AGZ_NS_END(Atrc)
