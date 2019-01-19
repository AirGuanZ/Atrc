#pragma once

#include <AGZUtils/Utils/Misc.h>

struct Global : public AGZ::Singleton<Global>
{
    int framebufferWidth;
    int framebufferHeight;

    float GetWindowAspectRatio() const noexcept
    {
        return static_cast<float>(framebufferWidth) / framebufferHeight;
    }
};
