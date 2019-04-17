#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define AGZ_USE_GLFW
#define AGZ_USE_OPENGL

#include <AGZUtils/Input/GLFWCapturer.h>
#include <AGZUtils/Utils/GL.h>

#include <Lib/imgui/imgui/imgui.h>
#include <Lib/imgui/imgui_impl_glfw.h>
#include <Lib/imgui/imgui_impl_opengl3.h>

using namespace AGZ::GraphicsAPI;
using GL::Mat4f;
using GL::Vec2f;
using GL::Vec3f;
using GL::Vec4f;
using GL::Vec2b;
using GL::Vec3b;
using GL::Vec4b;
using GL::Vec2i;
using GL::Vec3i;
using GL::Vec4i;
using GL::Rad;
using GL::Deg;

namespace ImGui
{
    inline void ShowTooltipForLastItem(const char *msg)
    {
        if(IsItemHovered())
        {
            BeginTooltip();
            TextUnformatted(msg);
            EndTooltip();
        }
    }
}
