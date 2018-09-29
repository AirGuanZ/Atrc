#pragma once

#include <Atrc/Common.h>
#include <Atrc/Math/Math.h>

AGZ_NS_BEG(Atrc)

ATRC_INTERFACE Camera
{
public:

    virtual ~Camera() = default;

    // Screen Sample: x: [-1, +1] y: [-1, +1]
    virtual Ray GetRay(const Vec2r &screenSample) const = 0;
};

AGZ_NS_END(Atrc)
