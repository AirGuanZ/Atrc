#pragma once

#include <QWidget>

#include <agz/editor/ui/utility/vec_input.h>

AGZ_EDITOR_BEGIN

class CameraPanel : public QWidget
{
public:
    
    explicit CameraPanel(QWidget *parent = nullptr);

    RealInput *distance = nullptr;
    Vec2Input *radian   = nullptr;
    Vec3Input *position = nullptr;
    Vec3Input *look_at  = nullptr;
    Vec3Input *up       = nullptr;

    RealInput *fov            = nullptr;
    RealInput *lens_radius    = nullptr;
    RealInput *focal_distance = nullptr;
};

AGZ_EDITOR_END
