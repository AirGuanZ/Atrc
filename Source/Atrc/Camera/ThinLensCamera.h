#pragma once

#include<Atrc/Core/Core.h>

AGZ_NS_BEG(Atrc)

// 见 https://airguanz.github.io/2018/12/02/camera-models.html 中的薄凸透镜模型
class ThinLensCamera : public CommonTransformCamera
{
    Real L_;
    Real lensRadius_, areaLens_;
    Real focalPlaneDistance_; // 焦平面距离而非焦距

public:

    static Real FocalPlaneDistance2FocalDistance(Real sensorLensDistance, Real focalPlaneDistance);

    ThinLensCamera(
        const Vec2 &filmSize, const Vec2 &sensorRectSize,
        const Vec3 &lensPos, const Vec3 &lookAt, const Vec3 &u,
        Real sensorLensDistance, Real lensRadius, Real focalDistance);

    CameraRay GetRay(const Vec2 &rasterPos) const override;
};

AGZ_NS_END(Atrc)
