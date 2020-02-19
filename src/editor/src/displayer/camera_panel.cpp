#include <QFormLayout>
#include <QLabel>

#include <agz/editor/displayer/camera_panel.h>

AGZ_EDITOR_BEGIN

CameraPanel::CameraPanel(QWidget *parent)
    : QWidget(parent)
{
    distance = new RealInput(this);
    radian   = new Vec2Input(this);
    position = new Vec3Input(this);
    look_at  = new Vec3Input(this);
    up       = new Vec3Input(this);

    fov            = new RealInput(this);
    lens_radius    = new RealInput(this);
    focal_distance = new RealInput(this);

    QFormLayout *layout = new QFormLayout(this);
    layout->addRow(new QLabel("Distance"),       distance);
    layout->addRow(new QLabel("Radian"),         radian);
    layout->addRow(new QLabel("Position"),       position);
    layout->addRow(new QLabel("LookAt"),         look_at);
    layout->addRow(new QLabel("Up"),             up);
    layout->addRow(new QLabel("FOV"),            fov);
    layout->addRow(new QLabel("Lens Size"),      lens_radius);
    layout->addRow(new QLabel("Focal Distance"), focal_distance);

    setContentsMargins(0, 0, 0, 0);
    layout->setContentsMargins(0, 0, 0, 0);
}

AGZ_EDITOR_END
