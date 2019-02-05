#pragma once

#include <Atrc/ModelViewer/Camera.h>
#include <Atrc/ModelViewer/GL.h>

class ScreenAxis
{
public:

    void Display(const Camera &camera, const GL::Immediate2D &imm);
};
