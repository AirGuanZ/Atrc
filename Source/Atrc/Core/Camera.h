#pragma once

#include <Atrc/Core/Common.h>
#include <Atrc/Core/Ray.h>

AGZ_NS_BEG(Atrc)

struct CameraRay
{
    Ray ray;
    Spectrum we;  // We^{(j)} * cos * area(Mj)
    Real pdf = 1; // 采样ray.dir的概率密度函数值
};

// 定义和符号含义见 https://airguanz.github.io/2018/12/02/camera-models.html
class Camera
{
protected:

    Real wI_, hI_;

public:

    Camera(Real wI, Real hI): wI_(wI), hI_(hI) { }

    virtual ~Camera() = default;

    // rasterPos为像素坐标系中的位置，
    virtual CameraRay GetRay(const Vec2 &rasterPos) const = 0;
};

class CommonTransformCamera : public Camera
{
protected:

    Real wS_, hS_; // 元器件矩形的宽高
    Real halfWS_, halfHS_;

    Mat3 R_;    // local2World 旋转矩阵
    Mat3 invR_; // world2Local 旋转矩阵
    Vec3 T_;    // local2World 平移量

    Vec2 Film2Sensor(const Vec2 &rasterPos) const;
    Vec2 Sensor2Film(const Vec2 &sensorPos) const;

    Vec3 LocalPoint2World(const Vec3 &local) const;
    Vec3 WorldPoint2Local(const Vec3 &world) const;
    Vec3 LocalVector2World(const Vec3 &local) const;
    Vec3 WorldVector2Local(const Vec3 &world) const;

public:

    CommonTransformCamera(
        const Vec2 &filmSize, const Vec2 &sensorRectSize,
        const Vec3 &centrePos, const Vec3 &lookAt, const Vec3 &u);
};

AGZ_NS_END(Atrc)
