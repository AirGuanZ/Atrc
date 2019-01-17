#pragma once

#define AGZ_USE_GLFW
#define AGZ_USE_OPENGL

#include <GLLibs/glew.h>
#include <GLLibs/glfw3.h>

#include <AGZUtils/Input/GLFWCapturer.h>
#include <AGZUtils/Utils/GL.h>

#include "GUI/imgui/imgui.h"
#include "GUI/imgui_impl_glfw.h"
#include "GUI/imgui_impl_opengl3.h"

using namespace AGZ::GraphicsAPI;
