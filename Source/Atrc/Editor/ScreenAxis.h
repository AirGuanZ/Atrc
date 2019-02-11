#pragma once

#include <Atrc/Editor/Camera.h>
#include <Atrc/Editor/GL.h>

class ScreenAxis
{
public:

    void Display(const Mat4f &projViewMat, const GL::Immediate2D &imm);
};
