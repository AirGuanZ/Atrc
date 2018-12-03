#pragma once

#include <Atrc/Core/Common.h>
#include <Atrc/Core/Ray.h>

AGZ_NS_BEG(Atrc)

// 定义和符号含义见 https://airguanz.github.io/2018/12/02/camera-models.html
class Camera
{
protected:

    Real wI_, hI_;

public:

    Camera(Real wI, Real hI): wI_(wI), hI_(hI) { }

    virtual ~Camera() = default;

    // rasterPos为像素坐标系中的位置，
    virtual Ray GetRay(const Vec2 &rasterPos) const = 0;
};

AGZ_NS_END(Atrc)
