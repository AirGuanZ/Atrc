#pragma once

#include <agz/rasterizer/common.h>

AGZ_RASTERIZER_BEGIN

struct VaryingBase
{
    Vec4 agz_position;
    real agz_depth = 0;
};

template<typename T>
using is_varying = std::is_base_of<VaryingBase, T>;

template<typename T>
constexpr bool is_varying_v = is_varying<T>::value;

AGZ_RASTERIZER_END
