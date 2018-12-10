#pragma once

#include <Atrc/Lib/Core/Camera.h>

AGZ_NS_BEG(Atrc)

class PinholeCamera : public Camera
{
    Real sensorWidth_, sensorHeight_; // 元器件矩形的宽高

    Mat3 R_; // local2World 旋转矩阵
    Vec3 T_; // local2World 平移量

    Real L_; // 小孔和元器件平面间的距离

public:

    PinholeCamera(
        uint32_t filmWidth, uint32_t filmHeight, const Vec2 &sensorRectSize,
        Real sensorPinholeDistance, const Vec3 &pinholePos, const Vec3 &lookAt, const Vec3 &u);
    
    GenerateRayResult GenerateRay(const Vec2 &rasterPos) const noexcept override;
};

AGZ_NS_END(Atrc)
