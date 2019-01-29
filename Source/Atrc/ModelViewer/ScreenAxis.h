#pragma once

#include "Camera.h"
#include "GL.h"

class ScreenAxis
{
public:

    void Display(const Camera &camera, const GL::Immediate &imm);
};
