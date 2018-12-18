#include <Atrc/Lib/Camera/PinholeCamera.h>

namespace Atrc
{

PinholeCamera::PinholeCamera(
    uint32_t filmWidth, uint32_t filmHeight, const Vec2 &sensorRectSize,
    Real sensorPinholeDistance, const Vec3 &pinholePos, const Vec3 &lookAt, const Vec3 &u)
    : Camera(filmWidth, filmHeight),
      sensorWidth_(sensorRectSize.x), sensorHeight_(sensorRectSize.y),
      T_(pinholePos), L_(sensorPinholeDistance)
{
    Vec3 ex = (lookAt - pinholePos).Normalize();
    Vec3 ey = Cross(u, ex).Normalize();
    Vec3 ez = Cross(ex, ey).Normalize();
    R_ = Mat3::FromCols(ex, ey, ez);
}

PinholeCamera::GenerateRayResult PinholeCamera::GenerateRay(const CameraSample &sample) const noexcept
{
    auto sensorPos = Vec2(
        sensorWidth_ *  (sample.film.x / filmWidth_ - Real(0.5)),
        sensorHeight_ * (Real(0.5) - sample.film.y / filmHeight_));
    return {
        Ray(T_, R_ * Vec3(L_, sensorPos.x, -sensorPos.y), EPS),
        Spectrum(1.0f), Real(1)
    };
}

} // namespace Atrc
