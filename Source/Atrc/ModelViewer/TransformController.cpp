#include <Lib/imgui/imgui/ImGuizmo.h>
#include "TransformController.h"

void TransformController::BeginFrame()
{
    ImGuizmo::BeginFrame();
}

void TransformController::EnableController()
{
    ImGuizmo::Enable(true);
}

void TransformController::DisableController()
{
    ImGuizmo::Enable(false);
}

void TransformController::SetCurrentMode(Mode mode) noexcept
{
    mode_ = mode;
}

TransformController::Mode TransformController::GetCurrentMode(Mode mode) const noexcept
{
    return mode_;
}

void TransformController::UseLocal(bool local) noexcept
{
    local_ = local;
}

void TransformController::Display(const Camera &camera, Vec3f *translate, Vec3f *rotate, float *scale) const
{
    // 设置imguizmo参数

    ImGuizmo::OPERATION guizmoOperation;
    switch(mode_)
    {
    case Translate: guizmoOperation = ImGuizmo::TRANSLATE; break;
    case Rotate:    guizmoOperation = ImGuizmo::ROTATE;    break;
    case Scale:     guizmoOperation = ImGuizmo::SCALE;     break;
    default:
        return;
    }

    ImGuizmo::MODE guizmoMode = local_ ? ImGuizmo::LOCAL : ImGuizmo::WORLD;

    // 构造model matrix，绘制controller并分解各变换分量

    const Mat4f &view = camera.GetViewMatrix();
    const Mat4f &proj = camera.GetProjMatrix();

    float worldMat[16];
    float scaleVec3[3] = { *scale, *scale, *scale };
    ImGuizmo::RecomposeMatrixFromComponents(&translate->x, &rotate->x, scaleVec3, worldMat);

    ImGuiIO &io = ImGui::GetIO();
    ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
    ImGuizmo::Manipulate(&view.m[0][0], &proj.m[0][0], guizmoOperation, guizmoMode, worldMat);

    ImGuizmo::DecomposeMatrixToComponents(worldMat, &translate->x, &rotate->x, scaleVec3);

    // 更新*scale

    int maxDeltaScaleIndex = -1;
    float maxDeltaScale = -1;
    for(int i = 0; i < 3; ++i)
    {
        float deltaScale = std::abs(scaleVec3[i] - *scale);
        if(deltaScale > maxDeltaScale)
        {
            maxDeltaScale = deltaScale;
            maxDeltaScaleIndex = i;
        }
    }
    *scale = scaleVec3[maxDeltaScaleIndex];
}
