#pragma once

#include <Atrc/Core/Core.h>

AGZ_NS_BEG(Atrc)

// 见 https://airguanz.github.io/2018/12/02/camera-models.html 中的Environment Camera
class EnvironmentCamera : public Camera
{
    Mat3 R_;
    Vec3 T_;

    Vec3 LocalPoint2World(const Vec3 &local) const;
    Vec3 LocalVector2World(const Vec3 &local) const;

public:

    EnvironmentCamera(
        const Vec2 &filmSize,
        const Vec3 &centrePos, const Vec3 &lookAt, const Vec3 &up);

    CameraRay GetRay(const Vec2 &rasterPos) const override;
};

AGZ_NS_END(Atrc)
