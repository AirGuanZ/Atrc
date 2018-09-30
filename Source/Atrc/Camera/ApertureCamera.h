#pragma once

#include <Atrc/Common.h>
#include <Atrc/Camera/Camera.h>
#include "PerspectiveCamera.h"

AGZ_NS_BEG(Atrc)

class ApertureCamera
    : ATRC_IMPLEMENTS Camera
{
    Vec3r eye_, scrCen_, dir_;
    Vec3r scrX_, scrY_;
    Vec3r scrXDir_, scrYDir_;
    Real apertureRadius_, focusDis_;

public:

    ApertureCamera(
        const Vec3r &eye, const Vec3r &_dir, const Vec3r &up,
        Radr FOVy, Real aspectRatio, Real apertureRadius, Real focusDis);

    Ray GetRay(const Vec2r &screenSample) const override;
};

AGZ_NS_END(Atrc)
