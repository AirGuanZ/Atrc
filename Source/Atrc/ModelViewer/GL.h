#pragma once

#define GLEW_STATIC

#include <GLLibs/glew.h>
#include <GLLibs/glfw3.h>

#define AGZ_USE_GLFW
#define AGZ_USE_OPENGL

#include <AGZUtils/Input/GLFWCapturer.h>
#include <AGZUtils/Utils/GL.h>

#include <Lib/imgui/imgui/imgui.h>
#include <Lib/imgui/imgui_impl_glfw.h>
#include <Lib/imgui/imgui_impl_opengl3.h>

using namespace AGZ::GraphicsAPI;
