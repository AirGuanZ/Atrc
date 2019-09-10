#pragma once

#include <memory>

#include "./opengl.h"

class EnvironmentLightManager
{
    std::shared_ptr<texture2d_t> env_tex_;
    ImGui::FileBrowser hdr_browser_;

    bool rev_env_u_;
    float env_scale_;
    float env_size_;
    float env_height_;

public:

    EnvironmentLightManager();

    std::shared_ptr<texture2d_t> get_tex();

    bool update();

    void set_shader_uniforms(program_t &prog) const;
};
