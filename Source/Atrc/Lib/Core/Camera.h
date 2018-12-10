#pragma once

#include <Atrc/Lib/Core/Ray.h>

namespace Atrc
{

// 定义和符号含义见 https://airguanz.github.io/2018/12/02/camera-models.html
class Camera
{
protected:

    uint32_t filmWidth_, filmHeight_;

public:

    Camera(uint32_t filmWidth, uint32_t filmHeight);

    virtual ~Camera() = default;

    struct GenerateRayResult
    {
        Ray r;
        Spectrum qe;
        Real pdf;
    };

    virtual GenerateRayResult GenerateRay(const Vec2 &imagePos) const noexcept = 0;
};

// ================================= Implementation

inline Camera::Camera(uint32_t filmWidth, uint32_t filmHeight)
    : filmWidth_(filmWidth), filmHeight_(filmHeight)
{

}

} // namespace Atrc
