#pragma once

#include <Atrc/ModelViewer/Camera.h>
#include <Atrc/ModelViewer/GL.h>

class TransformController
{
public:

    enum Mode
    {
        None      = 0,
        Translate = 1,
        Rotate    = 2,
        Scale     = 3,
    };

    static void BeginFrame();

    static void EnableController();

    static void DisableController();

    void SetCurrentMode(Mode mode) noexcept;

    Mode GetCurrentMode(Mode mode) const noexcept;

    void UseLocal(bool local) noexcept;

    void Render(const Mat4f &proj, const Mat4f &view, Vec3f *translate, Vec3f *rotate, float *scale) const;

    void Display();

private:

    Mode mode_  = Translate;
    bool local_ = true;
};

class Transform
{
    Vec3f trans_;
    Vec3f rotate_;
    float scale_ = 1;
    TransformController controller_;

public:

    void Display(const Mat4f &proj, const Mat4f &view)
    {
        controller_.Display();
        controller_.Render(proj, view, &trans_, &rotate_, &scale_);

        ImGui::InputFloat3("translate##input_translate", &trans_[0]);
        ImGui::InputFloat3("rotate##input_rotate", &rotate_[0]);
        ImGui::InputFloat("scale##input_scale", &scale_);
    }

    const Vec3f &GetTranslate() const noexcept { return trans_; }
    const Vec3f &GetRotate() const noexcept { return rotate_; }
    float GetScale() const noexcept { return scale_; }
};
