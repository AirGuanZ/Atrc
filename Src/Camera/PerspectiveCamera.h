#pragma once

#include "../Common.h"
#include "../Math/AGZMath.h"
#include "Camera.h"

AGZ_NS_BEG(Atrc)

class PerspectiveCamera : ATRC_IMPLEMENTS Camera
{
    Vec3r eye_, scrCen_;
    Vec3r scrX_, scrY_;

public:

    PerspectiveCamera(
        const Vec3r &eye, const Vec3r &_dir, const Vec3r &up,
        Radr FOVy, Real aspectRatio, Real distance);

    Ray Generate(const Vec2r &screenSample) const override;
};

AGZ_NS_END(Atrc)
