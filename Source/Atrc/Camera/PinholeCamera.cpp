#include <Atrc/Camera/PinholeCamera.h>

AGZ_NS_BEG(Atrc)

PinholeCamera::PinholeCamera(
    const Vec2 &filmSize, const Vec2 &sensorRectSize,
    Real sensorPinholeDistance, const Vec3 &pinholePos, const Vec3 &lookAt, const Vec3 &u)
    : Camera(filmSize.x, filmSize.y),
      wS_(sensorRectSize.x), hS_(sensorRectSize.y),
      halfWS_(sensorRectSize.x / 2), halfHS_(sensorRectSize.y / 2),
      L_(sensorPinholeDistance), T_(pinholePos)
{
    Vec3 ex = (lookAt - pinholePos).Normalize();
    Vec3 ey = Cross(u, ex).Normalize();
    Vec3 ez = Cross(ex, ey);
    R_ = Mat3::FromCols(ex, ey, ez);
}

Ray PinholeCamera::GetRay(const Vec2 &rasterPos) const
{
    Real xS = wS_ * (rasterPos.x / wI_) - halfWS_;
    Real yS = halfHS_ - hS_ * (rasterPos.y / hI_);

    Vec3 origin = T_;
    Vec3 direction = R_ * Vec3(L_, xS, -yS);

    return Ray(origin, direction.Normalize(), EPS);
}

AGZ_NS_END(Atrc)
