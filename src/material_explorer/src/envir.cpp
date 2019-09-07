#include <iostream>

#include <agz/utility/image.h>

#include "./envir.h"

namespace
{
    auto load_env_light(const std::string &filename)
    {
        auto ret = std::make_shared<texture2d_t>(true);
        auto tex_data = agz::img::load_rgb_from_hdr_file(filename);
        ret->initialize_format_and_data(
            1, GL_RGB32F, tex_data.shape()[1], tex_data.shape()[0], tex_data.raw_data());
        return ret;
    }
}

EnvironmentLightManager::EnvironmentLightManager()
{
    env_tex_ = load_env_light("./beach_parking_4k.hdr");
}

std::shared_ptr<texture2d_t> EnvironmentLightManager::get_tex()
{
    return env_tex_;
}

bool EnvironmentLightManager::update()
{
    ImGui::PushID(this);
    AGZ_SCOPE_GUARD({ ImGui::PopID(); });

    if(ImGui::Button("browse envir"))
        hdr_browser_.Open();

    hdr_browser_.Display();

    if(hdr_browser_.HasSelected())
    {
        auto filename = hdr_browser_.GetSelected();
        hdr_browser_.ClearSelected();
        env_tex_ = load_env_light(filename.string());
        std::cout << "load hdr env from " << filename << std::endl;
        return true;
    }

    return false;
}
