#include "./app.h"
#include "./diffuse.h"
#include "./disney.h"
#include "./glass.h"
#include "./mirror.h"

namespace
{
    std::unique_ptr<Material> create_material(const std::string &name)
    {
        if(name == "diffuse")
            return std::make_unique<Diffuse>();
        if(name == "disney")
            return std::make_unique<Disney>();
        if(name == "glass")
            return std::make_unique<Glass>();
        if(name == "mirror")
            return std::make_unique<Mirror>();
        return nullptr;
    }
}

App::App(int framebuffer_width, int framebuffer_height)
    : fb_width_(framebuffer_width), fb_height_(framebuffer_height), camera_(float(fb_width_) / fb_height_)
{
    mat_ = std::make_unique<MaterialHolder>(create_material("diffuse"), fb_width_, fb_height_);

    material_names_ = { "diffuse", "disney", "glass", "mirror" };
    current_material_name_ = "diffuse";
}

void App::update(GLFWwindow *window)
{
    if(camera_.show_gui() && mat_)
        mat_->restart();

    ImGui::Separator();

    if(env_light_.update() && mat_)
        mat_->restart();

    ImGui::Separator();

    if(ImGui::BeginCombo("material type", current_material_name_.c_str()))
    {
        AGZ_SCOPE_GUARD({ ImGui::EndCombo(); });

        bool changed = false;
        std::string new_mat_name;
        for(auto &mat_name : material_names_)
        {
            bool selected = mat_name == current_material_name_;
            if(ImGui::Selectable(mat_name.c_str(), selected))
            {
                changed = true;
                new_mat_name = mat_name;
            }
            if(selected)
                ImGui::SetItemDefaultFocus();
        }

        if(changed)
        {
            if(auto new_mat = create_material(new_mat_name))
            {
                current_material_name_ = new_mat_name;
                mat_ = std::make_unique<MaterialHolder>(std::move(new_mat), fb_width_, fb_height_);
            }
        }
    }

    ImGui::Separator();

    if(mat_)
        mat_->show_gui(window);

    if(!ImGui::IsAnyItemFocused())
    {
        if(camera_.update(float(clock_.us()) / 1000.0f))
            mat_->restart();
    }
}

void App::render()
{
    if(mat_)
        mat_->render(camera_, env_light_);
    clock_.restart();
}
