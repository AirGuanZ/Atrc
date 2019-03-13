#include <Atrc/Atrc/GL.h>
#include <Atrc/Atrc/Texture1/Constant1.h>

Constant1Core::Constant1Core(ResourceCreateContext&)
    : minValue_(1), maxValue_(-1), value_(0)
{
    
}

void Constant1Core::Display(ResourceCreateContext&)
{
    ImGui::PushID(this);
    ImGui::Text("texel");
    ImGui::SameLine();
    ImGui::PushID(0);
    ImGui::PushItemWidth(-1);

    if(minValue_ <= maxValue_)
        ImGui::SliderFloat("", &value_, minValue_, maxValue_);
    else
        ImGui::InputFloat("", &value_);
    
    ImGui::PopItemWidth();
    ImGui::PopID();
    ImGui::PopID();
}

void Constant1Core::SetRange(float minValue, float maxValue)
{
    minValue_ = minValue;
    maxValue_ = maxValue;
    if(minValue_ <= maxValue_)
        value_ = AGZ::Math::Clamp(value_, minValue_, maxValue_);
}
