#pragma once

#include "Camera.h"
#include "GL.h"

class TransformController
{
public:

    TransformController() noexcept;

    // 激活平移控制模式并设置binding
    void BindTranslation(Vec3f *transOffset) noexcept;

    // 激活旋转控制模式并设置binding
    void BindRotation(Vec3f *rotDegree) noexcept;

    // 激活缩放控制模式
    void BindScale(float *scaleSize) noexcept;

    // disable
    void Unbind() noexcept;

    // 绘制控制器
    void Display(const Camera &camera, const Mat4f &preTransform);

    // 根据用户输入更新数据
    void Update(const AGZ::Input::Mouse &mouse, const Camera &camera, const Mat4f &preTransform);

private:

    void ResetBindingData() noexcept;

    void ResetDraggingData() noexcept;

    void DisplayTranslate(const Camera &camera, const Mat4f &preTransform);

    void DisplayRotate(const Camera &camera, const Mat4f &preTransform);

    void DisplayScale(const Camera &camera, const Mat4f &preTransform);

    void UpdateTranslate(const AGZ::Input::Mouse &mouse, const Camera &camera, const Mat4f &preTransform);

    void UpdateRotate(const AGZ::Input::Mouse &mouse, const Camera &camera, const Mat4f &preTransform);

    void UpdateScale(const AGZ::Input::Mouse &mouse, const Camera &camera, const Mat4f &preTransform);

    // 以下三个指针指向数据binding，其中至多有一个不为空

    Vec3f *transOffset_;
    Vec3f *rotDegree_;
    float *scaleSize_;

    // 鼠标拖拽控制器时的起始点（屏幕空间）
    Vec2f dragOriginP_;
    // 鼠标是否在拖拽控制器（分别表示x，y，z，对scale模式只有isDragging_[0]有意义）
    bool isDragging_[3];
};
