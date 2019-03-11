#include <Atrc/Atrc/FilmFilter/Box.h>
#include <Atrc/Atrc/GL.h>

BoxCore::BoxCore(const ResourceCreateContext&)
    : sidelen_(1)
{
    
}

void BoxCore::Display(ResourceCreateContext&)
{
    ImGui::PushID(this);
    ImGui::Text("sidelen");
    ImGui::SameLine();
    ImGui::PushID(0);
    ImGui::PushItemWidth(-1);
    ImGui::InputFloat("", &sidelen_);
    ImGui::PopItemWidth();
    ImGui::PopID();
    ImGui::PopID();
}
