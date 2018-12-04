#include <Atrc/Camera/ThinLensCamera.h>

AGZ_NS_BEG(Atrc)

ThinLensCamera::ThinLensCamera(
    const Vec2 &filmSize, const Vec2 &sensorRectSize,
    const Vec3 &lensPos, const Vec3 &lookAt, const Vec3 &u,
    Real sensorLensDistance, Real aperture)
    : CommonTransformCamera(filmSize, sensorRectSize, lensPos, lookAt, u)
{

}

AGZ_NS_END(Atrc)
