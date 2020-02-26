#pragma once

#include <QPointer>

#include <agz/editor/displayer/camera_panel.h>
#include <agz/editor/displayer/gl_widget.h>

AGZ_EDITOR_BEGIN

class Editor;

class Displayer : public QWidget
{
    Q_OBJECT

public:

    Displayer(QWidget *parent, Editor *editor);

    void load_camera_from_config(const tracer::ConfigGroup &camera_params);

    std::shared_ptr<tracer::Camera> create_camera();

    CameraPanel *get_camera_panel();

    Vec3 get_camera_pos() const;

    Vec3 get_camera_dir() const;

    Vec3 get_camera_dst() const;

    Vec3 get_camera_up() const;

    real get_camera_fov_deg() const;

    Vec2i pixmap_size() const;

    void set_display_image(const Image2D<Spectrum> &img);

    bool is_in_realtime_mode() const;

    DisplayerGLWidget *get_gl_widget();

signals:

    void left_button_emit_ray(const Vec3 &o, const Vec3 &d);

    void need_to_recreate_camera();

    void update_camera_panel();

public slots:

    void set_camera_rotate_speed(real speed);

protected:

    void leaveEvent(QEvent *event) override;

    void resizeEvent(QResizeEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;

    void mouseMoveEvent(QMouseEvent *event) override;

    void mouseReleaseEvent(QMouseEvent *event) override;

    void wheelEvent(QWheelEvent *event) override;

private:

    static Vec2 pos_to_radian(const Vec3 &pos, const Vec3 &dst, const Vec3 &up);

    static Vec3 radian_to_pos(const Vec2 &radian, const Vec3 &dst, real distance, const Vec3 up);

    void update_gl_camera();

    DisplayerGLWidget *gl_widget_               = nullptr;

    struct DisplayerCameraParams
    {
        Vec2 radian   = { 0, PI_r / 2 };
        real distance = 4;

        Vec3 dst;
        Vec3 up = Vec3(0, 0, 1);

        real fov = 60;

        real lens_radius    = 0;
        real focal_distance = 1;
    };

    DisplayerCameraParams camera_params_;

    QPointer<CameraPanel> camera_panel_ = nullptr;

    // camera controller state

    enum class CameraState
    {
        Free,
        Rotate,
        Move

    } camera_state_ = CameraState::Free;

    Vec2i press_coord_;  // cursor coord when controller starts
    Vec2  press_radian_; // camera direction when controller starts
    Vec3  press_dst_;    // camera destination when controller starts

    real rotate_speed_  = real(0.0025);
    real panning_speed_ = real(0.001);
    
    Editor *editor_;
};

AGZ_EDITOR_END
