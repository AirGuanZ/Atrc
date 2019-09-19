#pragma once

#include <agz/rasterizer/common.h>

AGZ_RASTERIZER_BEGIN

struct DefaultDepthTester
{
    DepthBuffer *depth_buffer;

    static constexpr bool early_depth_test = true;

    bool process(int x, int y, real depth) const noexcept
    {
        auto &old_depth = depth_buffer->at(y, x);
        if(depth < old_depth)
        {
            old_depth = depth;
            return true;
        }
        return false;
    }
};

AGZ_RASTERIZER_END
