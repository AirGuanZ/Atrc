#pragma once

#include <QLabel>
#include <QPointer>

#include <agz/editor/displayer/camera_panel.h>

AGZ_EDITOR_BEGIN

class Displayer : public QLabel
{
    Q_OBJECT

public:

    explicit Displayer(QWidget *parent);

    void load_camera_from_config(const tracer::ConfigGroup &camera_params);

    std::shared_ptr<tracer::Camera> create_camera();

    CameraPanel *get_camera_panel();

signals:

    void need_to_recreate_camera();

protected:

    void resizeEvent(QResizeEvent *event) override;

private:

    struct DisplayerCameraParams
    {
        Vec3d pos = Vec3d(-5, 0, 0);
        Vec3d dst;
        Vec3d up = Vec3d(0, 0, 1);

        double fov = 60;

        double lens_radius = 0;
        double focal_distance = 1;
    };

    DisplayerCameraParams get_camera_params();

    QPointer<CameraPanel> camera_panel_ = nullptr;
};

AGZ_EDITOR_END
