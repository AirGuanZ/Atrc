#include <iostream>

#include <agz/utility/image.h>

#include "./envir.h"

namespace
{
    auto load_env_light(const std::string &filename)
    {
        const auto ret = std::make_shared<texture2d_t>(true);
        const auto tex_data = agz::img::load_rgb_from_hdr_file(filename);
        if(!tex_data.is_available())
            std::cout << "failed to load hdr env from " << filename << std::endl;
        else
            std::cout << "load hdr env from " << filename << std::endl;
        ret->initialize_format_and_data(
            1, GL_RGB32F, tex_data.shape()[1], tex_data.shape()[0], tex_data.raw_data());
        return ret;
    }
}

EnvironmentLightManager::EnvironmentLightManager()
    : rev_env_u_(true), env_scale_(1), env_size_(200), env_height_(0)
{
    env_tex_ = load_env_light("./beach_parking_4k_low_res.hdr");
}

std::shared_ptr<texture2d_t> EnvironmentLightManager::get_tex()
{
    return env_tex_;
}

bool EnvironmentLightManager::update()
{
    ImGui::PushID(this);
    AGZ_SCOPE_GUARD({ ImGui::PopID(); });
    bool ret = false;

    if(ImGui::Button("browse envir"))
        hdr_browser_.Open();

    hdr_browser_.Display();

    if(hdr_browser_.HasSelected())
    {
        const auto filename = hdr_browser_.GetSelected();
        hdr_browser_.ClearSelected();
        env_tex_ = load_env_light(filename.string());
        ret = true;
    }

    ImGui::SameLine();

    ret |= ImGui::Checkbox("rev env u", &rev_env_u_);
    ret |= ImGui::DragFloat("intensity", &env_scale_, 0.01f, 0, 3);

    ret |= ImGui::DragFloat("size", &env_size_, 0.01f);
    env_size_ = agz::math::clamp<float>(env_size_, 1.01f, 1000.0f);

    ret |= ImGui::DragFloat("height", &env_height_, 0.01f, -env_size_, env_size_);
    env_height_ = agz::math::clamp<float>(env_height_, -env_size_, env_size_);

    return ret;
}

void EnvironmentLightManager::set_shader_uniforms(program_t &prog) const
{
    prog.set_uniform_unchecked("envir_tex", 0);
    if(env_tex_)
        env_tex_->bind(0);
    else
        glBindTextureUnit(GL_TEXTURE0, 0);
    prog.set_uniform_unchecked("env_size", env_size_);
    prog.set_uniform_unchecked("env_height", env_height_);
    prog.set_uniform_unchecked("rev_env_u", rev_env_u_);
    prog.set_uniform_unchecked("env_scale", env_scale_);
}
