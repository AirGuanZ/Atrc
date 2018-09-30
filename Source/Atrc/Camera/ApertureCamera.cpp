#include <Atrc/Camera/ApertureCamera.h>

AGZ_NS_BEG(Atrc)

ApertureCamera::ApertureCamera(
    const Vec3r &eye, const Vec3r &_dir, const Vec3r &up,
    Radr FOVy, Real aspectRatio, Real apertureRadius, Real focusDis)
    : eye_(eye), scrCen_(AGZ::UNINITIALIZED), dir_(_dir.Normalize()),
      scrX_(AGZ::UNINITIALIZED), scrY_(AGZ::UNINITIALIZED),
      scrXDir_(AGZ::UNINITIALIZED), scrYDir_(AGZ::UNINITIALIZED),
      apertureRadius_(apertureRadius), focusDis_(focusDis)
{
    constexpr Real distance = 0.02;

    scrCen_ = eye_ + distance * dir_;

    scrXDir_ = Cross(dir_, up).Normalize();
    scrYDir_ = Cross(scrXDir_, dir_);

    Real scrYSize = distance * Tan(Real(0.5) * FOVy);
    Real scrXSize = scrYSize * aspectRatio;

    scrX_ = scrXSize * scrXDir_;
    scrY_ = scrYSize * scrYDir_;
}

namespace
{
    Vec2r UniformlySampleInUnitDisk()
    {
        for(;;)
        {
            Vec2r ret(2 * Rand() - 1, 2 * Rand() - 1);
            if(ret.LengthSquare() <= 1)
                return ret;
        }
    }
}

Ray ApertureCamera::GetRay(const Vec2r &screenSample) const
{
    Vec3r ori = scrCen_ + screenSample.x * scrX_ + screenSample.y * scrY_;
    Vec3r dir = (ori - eye_).Normalize();
    Real t = focusDis_ / Dot(dir, dir_);
    Vec3r focusPoint = eye_ + t * dir;

    Vec2r sam = apertureRadius_ * UniformlySampleInUnitDisk();
    Vec3r newOri = scrCen_ + scrXDir_ * sam.x + scrYDir_ * sam.y;

    return Ray(newOri, (focusPoint - newOri).Normalize());
}

AGZ_NS_END(Atrc)
