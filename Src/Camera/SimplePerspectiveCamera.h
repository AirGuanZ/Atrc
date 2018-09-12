#pragma once

#include "../Math/AGZMath.h"
#include "Camera.h"

AGZ_NS_BEG(Atrc)

class SimplePerspectiveCamera : public Camera
{
    Vec3r eye_, scrCen_;
    Vec3r scrX_, scrY_;

public:

    SimplePerspectiveCamera(
        const Vec3r &eye, const Vec3r &_dir, const Vec3r &up,
        Radr FOVy, Real aspectRatio, Real distance);

    Ray Generate(const Vec2r &screenSample) const override;
};

AGZ_NS_END(Atrc)
