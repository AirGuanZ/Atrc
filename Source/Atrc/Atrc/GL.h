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

#include <Lib/ImFileBrowser/imfilebrowser.h>

using namespace AGZ::GraphicsAPI;

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
