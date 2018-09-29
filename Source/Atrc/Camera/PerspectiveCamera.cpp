#include <Atrc/Camera/PerspectiveCamera.h>

AGZ_NS_BEG(Atrc)

PerspectiveCamera::PerspectiveCamera(
    const Vec3r &eye, const Vec3r &_dir, const Vec3r &up,
    Radr FOVy, Real aspectRatio, Real distance)
    : eye_(eye), scrCen_(AGZ::UNINITIALIZED),
      scrX_(AGZ::UNINITIALIZED), scrY_(AGZ::UNINITIALIZED)
{
    Vec3r dir = _dir.Normalize();
    scrCen_ = eye_ + distance * dir;

    Vec3r scrXDir = Cross(dir, up).Normalize();
    Vec3r scrYDir = Cross(scrXDir, dir);

    Real scrYSize = distance * Tan(Real(0.5) * FOVy);
    Real scrXSize = scrYSize * aspectRatio;

    scrX_ = scrXSize * scrXDir;
    scrY_ = scrYSize * scrYDir;
}

Ray PerspectiveCamera::Generate(const Vec2r &screenSample) const
{
    Vec3r ori = scrCen_ + screenSample.x * scrX_ + screenSample.y * scrY_;
    Vec3r dir = (ori - eye_).Normalize();
    return Ray(ori, dir);
}

AGZ_NS_END(Atrc)
