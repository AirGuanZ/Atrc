#include <Atrc/Editor/EntityController.h>
#include <Atrc/Mgr/Parser.h>
#include <Lib/imgui/imgui/ImGuizmo.h>

void EntityControllerAction::BeginFrame()
{
    ImGuizmo::BeginFrame();
}

void EntityControllerAction::EnableController()
{
    ImGuizmo::Enable(true);
}

void EntityControllerAction::DisableController()
{
    ImGuizmo::Enable(false);
}

void EntityControllerAction::SetCurrentMode(Mode mode) noexcept
{
    mode_ = mode;
}

EntityControllerAction::Mode EntityControllerAction::GetCurrentMode() const noexcept
{
    return mode_;
}

void EntityControllerAction::UseLocal(bool local) noexcept
{
    local_ = local;
}

void EntityControllerAction::Render(const Mat4f &proj, const Mat4f &view, Vec3f *translate, Vec3f *rotate, float *scale) const
{
    // 设置imguizmo参数

    if(mode_ == Edit)
        return;

    ImGuizmo::OPERATION guizmoOperation;
    switch(mode_)
    {
    case Translate: guizmoOperation = ImGuizmo::TRANSLATE; break;
    case Rotate:    guizmoOperation = ImGuizmo::ROTATE;    break;
    case Scale:     guizmoOperation = ImGuizmo::SCALE;     break;
    default: return;
    }

    ImGuizmo::MODE guizmoMode = local_ ? ImGuizmo::LOCAL : ImGuizmo::WORLD;

    // 构造model matrix，绘制controller并分解各变换分量

    float worldMat[16];
    float scaleVec3[3] = { *scale, *scale, *scale };
    ImGuizmo::RecomposeMatrixFromComponents(&translate->x, &rotate->x, scaleVec3, worldMat);

    ImGuizmo::Manipulate(&view.m[0][0], &proj.m[0][0], guizmoOperation, guizmoMode, worldMat);

    ImGuizmo::DecomposeMatrixToComponents(worldMat, &translate->x, &rotate->x, scaleVec3);

    // 更新*scale

    *scale = scaleVec3[0];
}

void EntityControllerAction::Display()
{
    int mode = static_cast<int>(mode_);

    ImGui::RadioButton("none",      &mode, static_cast<int>(None));      ImGui::SameLine();
    ImGui::RadioButton("translate", &mode, static_cast<int>(Translate)); ImGui::SameLine();
    ImGui::RadioButton("rotate",    &mode, static_cast<int>(Rotate));    ImGui::SameLine();
    ImGui::RadioButton("scale",     &mode, static_cast<int>(Scale));     ImGui::SameLine();
    ImGui::RadioButton("edit",      &mode, static_cast<int>(Edit));

    SetCurrentMode(static_cast<Mode>(mode));

    ImGui::Checkbox("local##use_local_transform_controller", &local_);
}

Mat4f EntityController::GetFinalMatrix() const noexcept
{
    float scaleVec[3] = { scale_, scale_, scale_ };
    Mat4f ret;
    ImGuizmo::RecomposeMatrixFromComponents(&trans_[0], &rotate_[0], scaleVec, &ret.m[0][0]);
    return ret;
}

void EntityController::Import(const AGZ::ConfigNode &node)
{
    auto transform = Atrc::Mgr::Parser::ParseTransform(node);
    auto realMat = transform.GetMatrix().Transpose();
    Mat4f mat;
    for(int c = 0; c != 4; ++c)
    {
        for(int r = 0; r != 4; ++r)
            mat.m[c][r] = float(realMat.m[c][r]);
    }
    float trans[3], rotate[3], scale[3];
    ImGuizmo::DecomposeMatrixToComponents(&mat.m[0][0], trans, rotate, scale);

    trans_  = Vec3f(trans);
    rotate_ = Vec3f(rotate);
    scale_  = scale[0];
}
