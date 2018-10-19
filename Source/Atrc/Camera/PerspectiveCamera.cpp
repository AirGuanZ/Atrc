#include <Atrc/Camera/PerspectiveCamera.h>

AGZ_NS_BEG(Atrc)

PerspectiveCamera::PerspectiveCamera(const Vec3 &eye, const Vec3 &_dir, const Vec3 &up, Rad FOVy, Real aspectRatio)
    : eye_(eye), scrCen_(UNINITIALIZED),
      scrX_(UNINITIALIZED), scrY_(UNINITIALIZED)
{
    Vec3 dir = _dir.Normalize();
    scrCen_ = eye_ + dir;

    Vec3 scrXDir = Cross(dir, up).Normalize();
    Vec3 scrYDir = Cross(scrXDir, dir);

    Real scrYSize = Tan(0.5 * FOVy);
    Real scrXSize = scrYSize * aspectRatio;

    scrX_ = scrXSize * scrXDir;
    scrY_ = scrYSize * scrYDir;
}

Ray PerspectiveCamera::GetRay(const Vec2 &screenSample) const
{
    Vec3 ori = scrCen_ + screenSample.x * scrX_ + screenSample.y * scrY_;
    Vec3 dir = (ori - eye_).Normalize();
    return Ray(ori, dir);
}

AGZ_NS_END(Atrc)
