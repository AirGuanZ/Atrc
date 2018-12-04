#pragma once

#include <Atrc/Core/Core.h>

AGZ_NS_BEG(Atrc)

class PinholeCamera : public CommonTransformCamera
{
    Real L_;

public:

    PinholeCamera(
        const Vec2 &filmSize, const Vec2 &sensorRectSize,
        Real sensorPinholeDistance, const Vec3 &pinholePos, const Vec3 &lookAt, const Vec3 &u);
    
    CameraRay GetRay(const Vec2 &rasterPos) const;
};

AGZ_NS_END(Atrc)
