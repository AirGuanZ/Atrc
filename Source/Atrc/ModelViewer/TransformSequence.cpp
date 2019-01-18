#include <AGZUtils/Utils/Input.h>

#include "TransformSequence.h"

Mat4f TransformSequence::Transform::GetMatrix() const noexcept
{
    return AGZ::TypeOpr::MatchVar(data,
        [&](const Translate &param) { return Mat4f::Translate(param.offset); },
        [&](const RotateX &param)   { return Mat4f::RotateX(param.angle); },
        [&](const RotateY &param)   { return Mat4f::RotateY(param.angle); },
        [&](const RotateZ &param)   { return Mat4f::RotateZ(param.angle); },
        [&](const Scale &param)     { return Mat4f::Scale(Vec3f(param.scale)); });
}

std::string TransformSequence::Transform::Text() const
{
    return AGZ::TypeOpr::MatchVar(data,
        [&](const Translate &param) { return "translate " + AGZ::ToStr8(param.offset).ToStdString(); },
        [&](const RotateX &param)   { return "rotateX " + std::to_string(param.angle.value); },
        [&](const RotateY &param)   { return "rotateY " + std::to_string(param.angle.value); },
        [&](const RotateZ &param)   { return "rotateZ " + std::to_string(param.angle.value); },
        [&](const Scale &param)     { return "scale " + std::to_string(param.scale); });
}

AGZ::Option<TransformSequence::Transform> TransformSequence::New(const char *title, bool newPopup) const
{
    if(!ImGui::BeginPopup(title, ImGuiWindowFlags_AlwaysAutoResize))
        return AGZ::None;

    static int selectedType = 0;

    static Vec3f translateOffset;
    static float rotateAngle = 0.0f;
    static float scaleValue = 1.0f;

    if(newPopup)
    {
        selectedType = 0;
        translateOffset = Vec3f();
        rotateAngle = 0.0f;
        scaleValue = 1.0f;
    }

    ImGui::RadioButton("translate", &selectedType, 0); ImGui::SameLine();
    ImGui::RadioButton("rotateX",   &selectedType, 1); ImGui::SameLine();
    ImGui::RadioButton("rotateY",   &selectedType, 2); ImGui::SameLine();
    ImGui::RadioButton("rotateZ",   &selectedType, 3); ImGui::SameLine();
    ImGui::RadioButton("scale",     &selectedType, 4);

    switch(selectedType)
    {
    case 0:
        ImGui::InputFloat("x offset", &translateOffset.x, 0.05f, 1.0f, "%.4f");
        ImGui::InputFloat("y offset", &translateOffset.y, 0.05f, 1.0f, "%.4f");
        ImGui::InputFloat("z offset", &translateOffset.z, 0.05f, 1.0f, "%.4f");
        break;
    case 1:
    case 2:
    case 3:
        ImGui::InputFloat("angle (degree)", &rotateAngle, 1.0f, 5.0f, "%.4f");
        break;
    case 4:
        ImGui::InputFloat("scale value", &scaleValue, 0.01f, 0.1f, "%.4f");
        scaleValue = std::max(scaleValue, 0.01f);
        break;
    default:
        AGZ::Unreachable();
    }

    bool ok = false, cancel = false;

    if(ImGui::GetIO().KeysDown[AGZ::Input::KEY_ENTER])
        ok = true;
    if(ImGui::Button("ok"))
        ok = true;

    if(ok)
    {
        ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
        switch(selectedType)
        {
        case 0: return Transform{ Translate{ translateOffset } };
        case 1: return Transform{ RotateX{ Deg(rotateAngle) } };
        case 2: return Transform{ RotateY{ Deg(rotateAngle) } };
        case 3: return Transform{ RotateZ{ Deg(rotateAngle) } };
        case 4: return Transform{ Scale{ scaleValue } };
        default: AGZ::Unreachable();
        }
    }

    ImGui::SameLine();

    if(ImGui::GetIO().KeysDown[AGZ::Input::KEY_ESCAPE])
        cancel = true;
    if(ImGui::Button("cancel"))
        cancel = true;

    if(cancel)
    {
        ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
        return AGZ::None;
    }

    ImGui::EndPopup();
    return AGZ::None;
}

void TransformSequence::EditTransformParam(Transform &trans) const
{
    AGZ::TypeOpr::MatchVar(trans.data,
        [](Translate &param)
    {
        ImGui::InputFloat("x offset", &param.offset.x, 0.05f, 1.0f, "%.4f");
        ImGui::InputFloat("y offset", &param.offset.y, 0.05f, 1.0f, "%.4f");
        ImGui::InputFloat("z offset", &param.offset.z, 0.05f, 1.0f, "%.4f");
    },
        [](RotateX &param)
    {
        ImGui::InputFloat("angle (degree)", &param.angle.value, 1.0f, 5.0f, "%.4f");
    },
        [](RotateY &param)
    {
        ImGui::InputFloat("angle (degree)", &param.angle.value, 1.0f, 5.0f, "%.4f");
    },
        [](RotateZ &param)
    {
        ImGui::InputFloat("angle (degree)", &param.angle.value, 1.0f, 5.0f, "%.4f");
    },
        [](Scale &param)
    {
        ImGui::InputFloat("scale value", &param.scale, 0.01f, 0.1f, "%.4f");
    });
}

Mat4f TransformSequence::GetFinalTransformMatrix() const noexcept
{
    Mat4f ret;
    for(auto &t : transforms_)
        ret = ret * t.GetMatrix();
    return ret;
}

void TransformSequence::Display()
{
    {
        bool newPopup = false;
        if(ImGui::Button("new front###NewFront"))
        {
            newPopup = true;
            ImGui::OpenPopup("New###NewTransformOnFront");
        }
        if(auto t = New("New###NewTransformOnFront", newPopup))
            transforms_.insert(transforms_.begin(), *t);
    }

    ImGui::SameLine();

    {
        bool newPopup = false;
        if(ImGui::Button("new back###NewBack"))
        {
            newPopup = true;
            ImGui::OpenPopup("New###NewTransformOnBack");
        }
        if(auto t = New("New###NewTransformOnBack", newPopup))
            transforms_.push_back(*t);
    }

    static int selectedIdx = -1;

    ImGui::SameLine();

    auto count = static_cast<int>(transforms_.size());
    if(ImGui::Button("delete") && selectedIdx >= 0)
    {
        transforms_.erase(transforms_.begin() + selectedIdx);
        --count;
        if(selectedIdx >= count)
            --selectedIdx;
    }

    ImGui::SameLine();

    if(ImGui::Button("up") && selectedIdx >= 1)
    {
        std::swap(transforms_[selectedIdx], transforms_[selectedIdx - 1]);
        --selectedIdx;
    }

    ImGui::SameLine();

    if(ImGui::Button("down") && 0 <= selectedIdx && selectedIdx < count - 1)
    {
        std::swap(transforms_[selectedIdx], transforms_[selectedIdx + 1]);
        ++selectedIdx;
    }

    ImGui::SameLine();

    if(ImGui::Button("template"))
    {
        transforms_.push_back({ Translate { Vec3f()   } });
        transforms_.push_back({ RotateY   { Deg(0.0f) } });
        transforms_.push_back({ RotateZ   { Deg(0.0f) } });
        transforms_.push_back({ RotateX   { Deg(0.0f) } });
        transforms_.push_back({ Scale     { 1.0f      } });
    }

    for(int i = 0; i < count; ++i)
    {
        ImGui::PushID(i);
        bool selected = i == selectedIdx;
        auto text = transforms_[i].Text();
        if(ImGui::Selectable(text.c_str(), selected))
        {
            if(selected)
                selectedIdx = -1;
            else
                selectedIdx = i;
        }
        ImGui::OpenPopupOnItemClick("edit");
        if(ImGui::BeginPopup("edit"))
        {
            EditTransformParam(transforms_[i]);
            ImGui::EndPopup();
        }
        ImGui::PopID();
    }
}

void TransformSequence::Clear()
{
    transforms_.clear();
}
