#include <Atrc/Camera/PinholeCamera.h>

AGZ_NS_BEG(Atrc)

PinholeCamera::PinholeCamera(
    const Vec2 &filmSize, const Vec2 &sensorRectSize,
    Real sensorPinholeDistance, const Vec3 &pinholePos, const Vec3 &lookAt, const Vec3 &u)
    : CommonTransformCamera(filmSize, sensorRectSize, pinholePos, lookAt, u),
      L_(sensorPinholeDistance)
{
    
}

CameraRay PinholeCamera::GetRay(const Vec2 &rasterPos) const
{
    auto sensorPos = Film2Sensor(rasterPos);
    return {
        Ray(LocalPoint2World(Vec3(Real(0))),
            LocalVector2World(Vec3(L_, sensorPos.x, -sensorPos.y)).Normalize(),
            EPS),
        Spectrum(1.0f), Real(1)
    };
}

AGZ_NS_END(Atrc)
