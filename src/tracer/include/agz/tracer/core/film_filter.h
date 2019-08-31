#pragma once

#include <agz/tracer_utility/math.h>
#include <agz/tracer_utility/object.h>

AGZ_TRACER_BEGIN

/**
 * @brief 图像重建滤波器
 * 
 * 参见 http://alvyray.com/Memos/CG/Microsoft/6_pixel.pdf
 */
class FilmFilter : public obj::Object
{
public:

    using Object::Object;

    /**
     * @brief 最大有效半径，以pixel为单位
     */
    virtual real radius() const noexcept = 0;

    /**
     * @brief 求滤波器值
     */
    virtual real eval(real x, real y) const noexcept = 0;
};

AGZT_INTERFACE(FilmFilter)

AGZ_TRACER_END
