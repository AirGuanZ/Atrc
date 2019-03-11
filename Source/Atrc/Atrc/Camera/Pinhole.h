#pragma once

#include <Atrc/Atrc/ResourceInterface/ResourceInstance.h>
#include <Atrc/Atrc/GL.h>

class PinholeCore
{
    Vec3f pos_;
    Vec3f lookAt_;
    Deg hori_, vert_;

    Deg FOVy_;
    float near_;
    float far_;

    Mat4f view_;
    Mat4f proj_;

public:

    static const char *GetTypeName() noexcept
    {
        return "Pinhole";
    }

    static bool IsMultiline() noexcept { return true; }

    explicit PinholeCore(ResourceCreateContext &ctx);

    void Display(ResourceCreateContext &ctx);
};
