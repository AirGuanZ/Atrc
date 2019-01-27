#include "Camera.h"
#include "GL.h"
#include "Global.h"

namespace
{
    // returns (hori, vert)
    std::pair<Deg, Deg> Dir2Angle(const Vec3f &dir)
    {
        if(!dir.Length())
            return { Deg(0), Deg(0) };
        Deg vert = Rad(AGZ::Math::Arcsin(dir.Normalize().y));
        Deg hori = dir.x || dir.z ? Rad(AGZ::Math::Arctan2(dir.z, dir.x)) : Rad(0);
        return { hori, vert };
    }

    Vec3f Angle2Dir(Deg hori, Deg vert)
    {
        float cos = Cos(vert);
        return Vec3f(cos * Cos(hori), Sin(vert), cos * Sin(hori));
    }
}

Camera::Camera(std::string name) noexcept
    : name_(std::move(name)), useLookAt_(false)
{
    auto &global = Global::GetInstance();
    projData_.w = static_cast<float>(global.framebufferWidth);
    projData_.h = static_cast<float>(global.framebufferHeight);
    UpdateViewData();
    UpdateViewMatrix();
    UpdateProjMatrix();
}

void Camera::Display()
{
    ImGui::PushID(this);
    AGZ::ScopeGuard cameraIDGuard([]() { ImGui::PopID(); });

    bool viewChanged = false, projChanged = false;

    if(ImGui::Button("autosize"))
    {
        auto &global = Global::GetInstance();
        projData_.w = static_cast<float>(global.framebufferWidth);
        projData_.h = static_cast<float>(global.framebufferHeight);
        projChanged = true;
    }
    ImGui::ShowTooltipForLastItem("use global framebuffer size as camera width/height");

    ImGui::SameLine();

    if(ImGui::Checkbox("lookat", &useLookAt_))
        UpdateViewData();
    ImGui::ShowTooltipForLastItem("use lookat instead hori/vert angle to specify camera direction");

    viewChanged |= ImGui::InputFloat3("position", &viewData_.pos[0]);

    if(useLookAt_)
    {
        viewChanged |= ImGui::InputFloat3("lookat pos", &viewData_.lookAt[0]);
    }
    else
    {
        viewChanged |= ImGui::InputFloat("hori angle", &viewData_.hori.value);
        viewChanged |= ImGui::InputFloat("vert angle", &viewData_.vert.value);
    }

    viewChanged |= ImGui::InputFloat("up angle", &viewData_.up.value);

    projChanged |= ImGui::InputFloat("width", &projData_.w);
    projChanged |= ImGui::InputFloat("height", &projData_.h);
    projChanged |= ImGui::InputFloat("FOVy", &projData_.FOVy.value);
    projChanged |= ImGui::InputFloat("near", &projData_.near);
    projChanged |= ImGui::InputFloat("far", &projData_.far);

    if(viewChanged)
        UpdateViewMatrix();

    if(projChanged)
        UpdateProjMatrix();
}

void Camera::UpdateViewData()
{
    if(useLookAt_)
    {
        auto angle = Dir2Angle(viewData_.lookAt - viewData_.pos);
        viewData_.hori = angle.first;
        viewData_.vert = angle.second;
    }
    else
    {
        auto dir = Angle2Dir(viewData_.hori, viewData_.vert);
        viewData_.lookAt = viewData_.pos + dir.Normalize();
    }
}

void Camera::UpdateViewMatrix()
{
    Vec3f dir;
    if(useLookAt_)
    {
        if(ApproxEq(viewData_.lookAt, viewData_.pos, 1e-3f))
            dir = Vec3f(1, 0, 0);
        else
            dir = viewData_.lookAt - viewData_.pos;
    }
    else
    {
        dir = Angle2Dir(viewData_.hori, viewData_.vert);
    }
    Vec3f up = !dir.x && !dir.z ? Vec3f::UNIT_X() : Vec3f::UNIT_Y();
    viewMat_ = Mat4f::LookAt(viewData_.pos, viewData_.pos + dir, up);
}

void Camera::UpdateProjMatrix()
{
    projMat_ = Mat4f::Perspective(projData_.FOVy, projData_.w / projData_.h, projData_.near, projData_.far);
}
