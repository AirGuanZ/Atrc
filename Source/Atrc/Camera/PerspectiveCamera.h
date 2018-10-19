#pragma once

#include <Atrc/Core/Core.h>

AGZ_NS_BEG(Atrc)

class PerspectiveCamera : public Camera
{
    Vec3 eye_, scrCen_;
    Vec3 scrX_, scrY_;

public:

    PerspectiveCamera(const Vec3 &eye, const Vec3 &_dir, const Vec3 &up, Rad FOVy, Real aspectRatio);

    Ray GetRay(const Vec2 &screenSample) const override;
};

AGZ_NS_END(Atrc)
