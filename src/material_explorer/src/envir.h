#pragma once

#include <memory>

#include "./opengl.h"

class EnvironmentLightManager
{
    std::shared_ptr<texture2d_t> env_tex_;
    ImGui::FileBrowser hdr_browser_;

public:

    EnvironmentLightManager();

    std::shared_ptr<texture2d_t> get_tex();

    bool update();
};
