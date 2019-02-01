#include "TransformController.h"
#include "ModelDataManager.h"

TransformController::TransformController() noexcept
    : transOffset_(nullptr), rotDegree_(nullptr), scaleSize_(nullptr),
      dragOriginP_(0.0f), isDragging_{false, false, false}
{
    
}

void TransformController::BindTranslation(Vec3f *transOffset) noexcept
{
    AGZ_ASSERT(transOffset);
    Unbind();
    transOffset_ = transOffset;
}

void TransformController::BindRotation(Vec3f *rotDegree) noexcept
{
    AGZ_ASSERT(rotDegree);
    Unbind();
    rotDegree_ = rotDegree;
}

void TransformController::BindScale(float *scaleSize) noexcept
{
    AGZ_ASSERT(scaleSize);
    Unbind();
    scaleSize_ = scaleSize;
}

void TransformController::Unbind() noexcept
{
    ResetBindingData();
    ResetDraggingData();
}

void TransformController::Display(const Camera &camera, const Mat4f &preTransform)
{
    if(transOffset_)
        DisplayTranslate(camera, preTransform);
    else if(rotDegree_)
        DisplayRotate(camera, preTransform);
    else if(scaleSize_)
        DisplayScale(camera, preTransform);
}

void TransformController::Update(const AGZ::Input::Mouse &mouse, const Camera &camera, const Mat4f &preTransform)
{
    if(transOffset_)
        UpdateTranslate(mouse, camera, preTransform);
    else if(rotDegree_)
        UpdateRotate(mouse, camera, preTransform);
    else if(scaleSize_)
        UpdateScale(mouse, camera, preTransform);
}

void TransformController::ResetBindingData() noexcept
{
    transOffset_ = nullptr;
    rotDegree_   = nullptr;
    scaleSize_   = nullptr;
}

void TransformController::ResetDraggingData() noexcept
{
    dragOriginP_ = Vec2f();
    for(int i = 0; i < 3; ++i)
        isDragging_[i] = false;
}

void TransformController::DisplayTranslate(const Camera &camera, const Mat4f &preTransform)
{
    AGZ_ASSERT(transOffset_);
    // TODO
}

void TransformController::DisplayRotate(const Camera &camera, const Mat4f &preTransform)
{
    
}

void TransformController::DisplayScale(const Camera &camera, const Mat4f &preTransform)
{
    
}

void TransformController::UpdateTranslate(const AGZ::Input::Mouse &mouse, const Camera &camera, const Mat4f& preTransform)
{
    
}

void TransformController::UpdateRotate(const AGZ::Input::Mouse &mouse, const Camera &camera, const Mat4f& preTransform)
{

}

void TransformController::UpdateScale(const AGZ::Input::Mouse &mouse, const Camera &camera, const Mat4f& preTransform)
{

}
