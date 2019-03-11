#include <Atrc/Atrc/FilmFilter/Gaussian.h>
#include <Atrc/Atrc/GL.h>

GaussianCore::GaussianCore(const ResourceCreateContext&)
    : radius_(0.5f), alpha_(2)
{

}

void GaussianCore::Display(ResourceCreateContext&)
{
    ImGui::PushID(this);

    ImGui::Text("radius"); ImGui::SameLine();
    ImGui::PushItemWidth(-1); ImGui::PushID(0); ImGui::InputFloat("", &radius_); ImGui::PopID(); ImGui::PopItemWidth();

    ImGui::Text("alpha "); ImGui::SameLine();
    ImGui::PushItemWidth(-1); ImGui::PushID(1); ImGui::InputFloat("", &alpha_); ImGui::PopID(); ImGui::PopItemWidth();

    ImGui::PopID();
}
