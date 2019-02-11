#pragma once

#include <Atrc/Editor/GL.h>

class DefaultRenderingCamera
{
public:

    // 和View Matrix相关的数据
    // hori + vert与lookAt都是用来管理摄像机视角的，具体在display界面显示哪个由useLookAt决定。
    // 随时保证两者是一致的。
    struct ViewData
    {
        Vec3f pos      = Vec3f(10.0f);
        Deg hori       = Deg(0);
        Deg vert       = Deg(0);
        Vec3f lookAt   = Vec3f(0.0f);;
        bool useLookAt = false;
        Deg up         = Deg(0);
    };

    // 和Projection Matrix相关的数据
    struct ProjData
    {
        int w      = 1000;
        int h      = 1000;
        Deg FOVy   = Deg(60);
        float near = 0.1f;
        float far  = 1000.0f;;
    };

    explicit DefaultRenderingCamera(std::string name) noexcept;

    const ViewData &GetViewData() const noexcept { return viewData_; }
    const ProjData &GetProjData() const noexcept { return projData_; }
    void SetViewData(const ViewData &viewData) noexcept { viewData_ = viewData; UpdateViewMatrix(); }
    void SetProjData(const ProjData &projData) noexcept { projData_ = projData; UpdateProjMatrix(); }

    const Mat4f &GetViewMatrix() const noexcept { return viewMat_; }
    const Mat4f &GetProjMatrix() const noexcept { return projMat_; }

    void Display();
    void UpdatePositionAndDirection(const AGZ::Input::Keyboard &kb, const AGZ::Input::Mouse &m);

    const std::string &GetName() const noexcept { return name_; }

    const Vec3f &GetPosition() const noexcept { return viewData_.pos; }
    const Vec3f &GetLookAt() const noexcept { return viewData_.lookAt; }

    int GetProjWidth() const noexcept { return projData_.w; }
    int GetProjHeight() const noexcept { return projData_.h; }
    Deg GetProjFOVy() const noexcept { return projData_.FOVy; }

    void AutoResizeProj();

    void UpdateMatrix()
    {
        UpdateViewData();
        UpdateViewMatrix();
        UpdateProjMatrix();
    }

private:

    Vec3f GetDirection() const;
    void UpdateViewData();

    void UpdateViewMatrix();
    void UpdateProjMatrix();

    std::string name_;

    ViewData viewData_;
    ProjData projData_;

    Mat4f viewMat_;
    Mat4f projMat_;
};
