#pragma once

#include <AGZUtils/Utils/Input.h>

#include "GL.h"

class Camera
{
public:

    struct ViewData
    {
        Vec3f pos    = Vec3f(10.0f);
        Deg hori     = Deg(0);
        Deg vert     = Deg(0);
        Vec3f lookAt = Vec3f(0.0f);;
        Deg up       = Deg(0);
    };

    struct ProjData
    {
        float w    = 1000.0f;
        float h    = 1000.0f;
        Deg FOVy   = Deg(60);
        float near = 0.1f;
        float far  = 1000.0f;;
    };

    explicit Camera(std::string name) noexcept;

    const ViewData &GetViewData() const noexcept { return viewData_; }
    const ProjData &GetProjData() const noexcept { return projData_; }
    void SetViewData(const ViewData &viewData) noexcept { viewData_ = viewData; UpdateViewMatrix(); }
    void SetProjData(const ProjData &projData) noexcept { projData_ = projData; UpdateProjMatrix(); }

    const Mat4f &GetViewMatrix() const noexcept { return viewMat_; }
    const Mat4f &GetProjMatrix() const noexcept { return projMat_; }

    void Display();
    void UpdatePositionAndDirection(const AGZ::Input::Keyboard &kb, const AGZ::Input::Mouse &m);

    const std::string &GetName() const noexcept { return name_; }

private:

    Vec3f GetDirection() const;
    void UpdateViewData();

    void UpdateViewMatrix();
    void UpdateProjMatrix();

    std::string name_;

    bool useLookAt_;

    ViewData viewData_;
    ProjData projData_;

    Mat4f viewMat_;
    Mat4f projMat_;
};
