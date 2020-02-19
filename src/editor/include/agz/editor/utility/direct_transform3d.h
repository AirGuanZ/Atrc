#pragma once

#include <agz/editor/common.h>

AGZ_EDITOR_BEGIN

struct DirectTransform
{
    Vec3 translate;
    Mat3 rotate = Mat3::identity();
    real scale = 1;

    static Vec3 to_euler_zyx(const Mat3 &rot) noexcept;

    static Mat3 from_euler_zyx(const Vec3 &rad) noexcept;

    Mat4 compose() const noexcept;
};

AGZ_EDITOR_END
