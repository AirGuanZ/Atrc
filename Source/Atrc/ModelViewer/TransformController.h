#pragma once

#include "Camera.h"
#include "GL.h"

class TransformController
{
public:

    enum Mode
    {
        None      = 0,
        Translate = 1,
        Rotate    = 2,
        Scale     = 3,
    };

    static void BeginFrame();

    static void EnableController();

    static void DisableController();

    void SetCurrentMode(Mode mode) noexcept;

    Mode GetCurrentMode(Mode mode) const noexcept;

    void UseLocal(bool local) noexcept;

    void Render(const Camera &camera, Vec3f *translate, Vec3f *rotate, float *scale) const;

    void Display();

private:

    Mode mode_  = Translate;
    bool local_ = true;
};
