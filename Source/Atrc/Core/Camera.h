#pragma once

#include <Atrc/Core/Common.h>
#include <Atrc/Core/Ray.h>

AGZ_NS_BEG(Atrc)

class Camera
{
public:

    virtual ~Camera() = default;

    // x in [-1, 1], y in [-1, 1]
    virtual Ray GetRay(const Vec2 &scrPos) const = 0;
};

AGZ_NS_END(Atrc)
