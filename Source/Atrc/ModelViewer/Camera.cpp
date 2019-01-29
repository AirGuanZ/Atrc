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

} // namespace null

Camera::Camera(std::string name) noexcept
    : name_(std::move(name))
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

    ImGui::Checkbox("lookat", &viewData_.useLookAt);
    UpdateViewData();

    ImGui::ShowTooltipForLastItem("use lookat instead hori/vert angle to specify camera direction");

    viewChanged |= ImGui::InputFloat3("position", &viewData_.pos[0]);

    if(viewData_.useLookAt)
    {
        viewChanged |= ImGui::InputFloat3("lookat pos", &viewData_.lookAt[0]);
    }
    else
    {
        viewChanged |= ImGui::InputFloat("hori angle", &viewData_.hori.value);
        viewChanged |= ImGui::InputFloat("vert angle", &viewData_.vert.value);
    }

    viewChanged |= ImGui::InputFloat("up angle", &viewData_.up.value);

    projChanged |= ImGui::InputFloat("width",  &projData_.w);
    projChanged |= ImGui::InputFloat("height", &projData_.h);
    projChanged |= ImGui::InputFloat("FOVy",   &projData_.FOVy.value);
    projChanged |= ImGui::InputFloat("near",   &projData_.near);
    projChanged |= ImGui::InputFloat("far",    &projData_.far);

    if(viewChanged)
        UpdateViewMatrix();

    if(projChanged)
        UpdateProjMatrix();
}

namespace
{
    Vec3f WASDDirection(const AGZ::Input::Keyboard &kb, const Vec3f &dir)
    {
        Vec3f moveDir;
        Vec3f horiDir = Vec3f(dir.x, 0, dir.z);
        if(horiDir.Length() > 1e-4f)
            horiDir = horiDir.Normalize();

        if(kb.IsKeyPressed('W'))
            moveDir += horiDir;
        if(kb.IsKeyPressed('A'))
            moveDir += Vec3f(-dir.z, 0, dir.x);
        if(kb.IsKeyPressed('S'))
            moveDir -= horiDir;
        if(kb.IsKeyPressed('D'))
            moveDir += Vec3f(dir.z, 0, -dir.x);

        moveDir *= 1.25f;

        if(kb.IsKeyPressed(AGZ::Input::KEY_SPACE))
            moveDir.y += 1;
        if(kb.IsKeyPressed(AGZ::Input::KEY_LSHIFT))
            moveDir.y -= 1;

        return moveDir;
    }
}

void Camera::UpdatePositionAndDirection(const AGZ::Input::Keyboard &kb, const AGZ::Input::Mouse &m)
{
    // wasd进行移动，space上升，shift下降
    // 如果开启了lookAt模式，就在平时移动摄像机，按住鼠标中键时移动lookAtPos
    // 否则，按住鼠标中键时直接按鼠标的移动来改变摄像机角度

    constexpr float MOVE_SPEED = 0.05f;

    Vec3f dir = GetDirection();

    if(!viewData_.useLookAt || !m.IsMouseButtonPressed(AGZ::Input::MOUSE_MIDDLE))
        viewData_.pos += MOVE_SPEED * WASDDirection(kb, dir);

    if(viewData_.useLookAt && m.IsMouseButtonPressed(AGZ::Input::MOUSE_MIDDLE))
        viewData_.lookAt += MOVE_SPEED * WASDDirection(kb, dir);

    if(!viewData_.useLookAt && m.IsMouseButtonPressed(AGZ::Input::MOUSE_MIDDLE))
    {
        viewData_.hori.value -= 0.15f * static_cast<float>(m.GetRelativeCursorPositionX());
        viewData_.vert.value -= 0.15f * static_cast<float>(m.GetRelativeCursorPositionY());

        viewData_.vert.value = AGZ::Math::Clamp(
            viewData_.vert.value,
            -Deg(0.5f * 0.98f * AGZ::Math::PI<Rad>).value,
            Deg(0.5f * 0.98f * AGZ::Math::PI<Rad>).value);
    }

    UpdateViewData();
    UpdateViewMatrix();
}

void Camera::UpdateViewData()
{
    if(viewData_.useLookAt)
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

Vec3f Camera::GetDirection() const
{
    Vec3f ret;

    if(viewData_.useLookAt)
    {
        if(ApproxEq(viewData_.lookAt, viewData_.pos, 1e-3f))
            ret = Vec3f(1, 0, 0);
        else
            ret = viewData_.lookAt - viewData_.pos;
    }
    else
        ret = Angle2Dir(viewData_.hori, viewData_.vert);

    return ret.Normalize();
}

void Camera::UpdateViewMatrix()
{
    Vec3f dir = GetDirection();
    Vec3f up = (dir.x == 0.0f && dir.z == 0.0f) ? Vec3f::UNIT_X() : Vec3f::UNIT_Y();
    viewMat_ = Mat4f::LookAt(viewData_.pos, viewData_.pos + dir, up);
}

void Camera::UpdateProjMatrix()
{
    projMat_ = Mat4f::Perspective(projData_.FOVy, projData_.w / projData_.h, projData_.near, projData_.far);
}
