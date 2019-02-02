#pragma once

#include "Camera.h"
#include "GL.h"

class TransformController
{
public:

    enum Mode
    {
        Translate,
        Rotate,
        Scale,
        None,
    };

    static void BeginFrame();

    static void EnableController();

    static void DisableController();

    void SetCurrentMode(Mode mode) noexcept;

    Mode GetCurrentMode(Mode mode) const noexcept;

    void UseLocal(bool local) noexcept;

    void Display(const Camera &camera, Vec3f *translate, Vec3f *rotate, float *scale) const;

private:

    Mode mode_  = None;
    bool local_ = true;
};
