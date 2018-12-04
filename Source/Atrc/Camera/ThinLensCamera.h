#pragma once

#include<Atrc/Core/Core.h>

AGZ_NS_BEG(Atrc)

class ThinLensCamera : public CommonTransformCamera
{
public:

    ThinLensCamera(
        const Vec2 &filmSize, const Vec2 &sensorRectSize,
        const Vec3 &lensPos, const Vec3 &lookAt, const Vec3 &u,
        Real sensorLensDistance, Real aperture);

    CameraRay GetRay(const Vec2 &rasterPos) const override;
};

AGZ_NS_END(Atrc)
