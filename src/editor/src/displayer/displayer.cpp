#include <QOpenGLWidget>
#include <QWheelEvent>

#include <agz/editor/displayer/camera_panel.h>
#include <agz/editor/displayer/displayer.h>
#include <agz/editor/editor.h>

AGZ_EDITOR_BEGIN

Displayer::Displayer(QWidget *parent, Editor *editor)
    : QWidget(parent), editor_(editor)
{
    gl_widget_ = new DisplayerGLWidget;
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(gl_widget_);

    setContentsMargins(0, 0, 0, 0);
    layout->setContentsMargins(0, 0, 0, 0);

    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    gl_widget_->setMouseTracking(true);
    setMouseTracking(true);

    update_gl_camera();
}

void Displayer::load_camera_from_config(const tracer::ConfigGroup &params)
{
    const Vec3 pos = params.child_vec3("pos");
    
    camera_params_.dst            = params.child_vec3("dst");
    camera_params_.up             = params.child_vec3("up");
    camera_params_.distance       = distance(pos, camera_params_.dst);
    camera_params_.radian         = pos_to_radian(pos, camera_params_.dst, camera_params_.up);
    camera_params_.fov            = params.child_real("fov");
    camera_params_.lens_radius    = params.child_real_or("lens_radius", 0);
    camera_params_.focal_distance = params.child_real_or("focal_distance", 1);
    
    emit update_camera_panel();
    emit need_to_recreate_camera();
}

std::shared_ptr<tracer::Camera> Displayer::create_camera()
{
    const int film_width = size().width();
    const int film_height = size().height();
    const real film_aspect = static_cast<real>(film_width) / film_height;

    const auto up_coord = tracer::Coord::from_z(camera_params_.up);
    const Vec3 local_dir = {
        std::sin(camera_params_.radian.y) * std::cos(camera_params_.radian.x),
        std::sin(camera_params_.radian.y) * std::sin(camera_params_.radian.x),
        std::cos(camera_params_.radian.y)
    };
    const Vec3 global_dir = up_coord.local_to_global(local_dir);
    const Vec3 pos = camera_params_.dst + global_dir * camera_params_.distance;

    return tracer::create_thin_lens_camera(
        film_aspect, pos, camera_params_.dst, camera_params_.up,
        math::deg2rad(real(camera_params_.fov)),
        real(camera_params_.lens_radius),
        real(camera_params_.focal_distance));
}

CameraPanel *Displayer::get_camera_panel()
{
    if(camera_panel_)
        return camera_panel_;

    camera_panel_ = new CameraPanel(this);

    connect(camera_panel_->distance, &RealInput::edit_value, [=](real new_distance)
    {
        if(new_distance < real(0.01))
        {
            camera_panel_->distance->set_value(real(0.01));
            new_distance = real(0.01);
        }

        camera_params_.distance = new_distance;

        AGZ_INFO("edit camera distance: {}", new_distance);

        update_gl_camera();
        emit update_camera_panel();
        emit need_to_recreate_camera();
    });

    connect(camera_panel_->position, &Vec3Input::edit_value, [=](const Vec3 &new_pos)
    {
        camera_params_.radian   = pos_to_radian(new_pos, camera_params_.dst, camera_params_.up);
        camera_params_.distance = distance(new_pos, camera_params_.dst);

        AGZ_INFO("edit camera position: {}, {}, {}", new_pos.x, new_pos.y, new_pos.z);

        update_gl_camera();
        emit update_camera_panel();
        emit need_to_recreate_camera();
    });

    connect(camera_panel_->look_at, &Vec3Input::edit_value, [=](const Vec3 &new_dst)
    {
        const Vec3 old_pos = radian_to_pos(
            camera_params_.radian, camera_params_.dst, camera_params_.distance, camera_params_.up);
        camera_params_.dst      = new_dst;
        camera_params_.radian   = pos_to_radian(old_pos, new_dst, camera_params_.up);
        camera_params_.distance = distance(old_pos, new_dst);

        AGZ_INFO("edit camera look_at: {}, {}, {}", new_dst.x, new_dst.y, new_dst.z);

        update_gl_camera();
        emit update_camera_panel();
        emit need_to_recreate_camera();
    });

    connect(camera_panel_->up, &Vec3Input::edit_value, [=](const Vec3 &new_up)
    {
        const Vec3 old_pos = radian_to_pos(
            camera_params_.radian, camera_params_.dst, camera_params_.distance, camera_params_.up);
        camera_params_.up     = new_up;
        camera_params_.radian = pos_to_radian(old_pos, camera_params_.dst, new_up);

        AGZ_INFO("edit camera up: {}, {}, {}", new_up.x, new_up.y, new_up.z);

        update_gl_camera();
        emit update_camera_panel();
        emit need_to_recreate_camera();
    });

    connect(camera_panel_->radian, &Vec2Input::edit_value, [=](const Vec2 &new_radian)
    {
        camera_params_.radian = new_radian;

        AGZ_INFO("edit camera direction: {}, {}", new_radian.x, new_radian.y);

        update_gl_camera();
        emit update_camera_panel();
        emit need_to_recreate_camera();
    });

    connect(camera_panel_->fov, &RealInput::edit_value, [=](real new_fov)
    {
        camera_params_.fov = new_fov;

        AGZ_INFO("edit camera fov: {}", new_fov);

        emit need_to_recreate_camera();
    });

    connect(camera_panel_->lens_radius, &RealInput::edit_value, [=](real lens_size)
    {
        camera_params_.lens_radius = lens_size;

        AGZ_INFO("edit camera lens radius: {}", lens_size);

        emit need_to_recreate_camera();
    });

    connect(camera_panel_->focal_distance, &RealInput::edit_value, [=](real new_focal_distance)
    {
        camera_params_.focal_distance = new_focal_distance;

        AGZ_INFO("edit camera focal distance: {}", new_focal_distance);

        emit need_to_recreate_camera();
    });

    connect(this, &Displayer::update_camera_panel, [=]
    {
        const Vec3 pos = radian_to_pos(
            camera_params_.radian, camera_params_.dst, camera_params_.distance, camera_params_.up);

        camera_panel_->distance      ->set_value(camera_params_.distance);
        camera_panel_->radian        ->set_value(camera_params_.radian);
        camera_panel_->position      ->set_value(pos);
        camera_panel_->look_at       ->set_value(camera_params_.dst);
        camera_panel_->up            ->set_value(camera_params_.up);
        camera_panel_->fov           ->set_value(camera_params_.fov);
        camera_panel_->lens_radius   ->set_value(camera_params_.lens_radius);
        camera_panel_->focal_distance->set_value(camera_params_.focal_distance);
    });

    emit update_camera_panel();
    return camera_panel_;
}

Vec3 Displayer::get_camera_pos() const
{
    const Vec3 pos = radian_to_pos(
        camera_params_.radian, camera_params_.dst,
        camera_params_.distance, camera_params_.up);
    return pos;
}

Vec3 Displayer::get_camera_dir() const
{
    const auto up_coord = tracer::Coord::from_z(camera_params_.up);
    const Vec3 local_dir = {
        std::sin(camera_params_.radian.y) * std::cos(camera_params_.radian.x),
        std::sin(camera_params_.radian.y) * std::sin(camera_params_.radian.x),
        std::cos(camera_params_.radian.y)
    };
    const Vec3 global_dir = up_coord.local_to_global(local_dir);
    return -global_dir;
}

Vec3 Displayer::get_camera_dst() const
{
    return camera_params_.dst;
}

Vec3 Displayer::get_camera_up() const
{
    return camera_params_.up;
}

real Displayer::get_camera_fov_deg() const
{
    return camera_params_.fov;
}

Vec2i Displayer::pixmap_size() const
{
    return { gl_widget_->width(), gl_widget_->height() };
}

void Displayer::set_display_image(const QImage &img)
{
    gl_widget_->set_background_image(img);
}

bool Displayer::is_in_realtime_mode() const
{
    return gl_widget_->is_in_realtime_mode();
}

DisplayerGLWidget *Displayer::get_gl_widget()
{
    return gl_widget_;
}

void Displayer::set_camera_rotate_speed(real speed)
{
    rotate_speed_ = speed;
}

void Displayer::leaveEvent(QEvent *event)
{
    gl_widget_->cursor_leave(event);
    camera_state_ = CameraState::Free;
}

void Displayer::resizeEvent(QResizeEvent *event)
{
    update_gl_camera();
    emit need_to_recreate_camera();
}

void Displayer::mousePressEvent(QMouseEvent *event)
{
    gl_widget_->mouse_press(event);

    if(event->button() == Qt::MouseButton::RightButton)
    {
        camera_state_ = CameraState::Rotate;
        press_coord_  = { event->x(), event->y() };
        press_radian_ = camera_params_.radian;
        press_dst_    = camera_params_.dst;
        return;
    }

    if(event->button() == Qt::MouseButton::MiddleButton)
    {
        camera_state_ = CameraState::Move;
        press_coord_  = { event->x(), event->y() };
        press_radian_ = camera_params_.radian;
        press_dst_    = camera_params_.dst;
    }
}

void Displayer::mouseMoveEvent(QMouseEvent *event)
{
    gl_widget_->mouse_move(event);

    if(camera_state_ == CameraState::Rotate)
    {
        const int dx = event->x() - press_coord_.x;
        const int dy = event->y() - press_coord_.y;

        camera_params_.radian.x = press_radian_.x - rotate_speed_ * real(dx);
        camera_params_.radian.y = math::clamp<real>(
            press_radian_.y - rotate_speed_ * dy, real(0.001), PI_r - real(0.001));

        update_gl_camera();
        editor_->on_change_camera();
        emit update_camera_panel();
        return;
    }

    if(camera_state_ == CameraState::Move)
    {
        const int dx = event->x() - press_coord_.x;
        const int dy = event->y() - press_coord_.y;

        const auto up_coord = tracer::Coord::from_z(camera_params_.up);
        const Vec3 local_dir = {
            std::sin(camera_params_.radian.y) * std::cos(camera_params_.radian.x),
            std::sin(camera_params_.radian.y) * std::sin(camera_params_.radian.x),
            std::cos(camera_params_.radian.y)
        };
        const Vec3 dst_to_pos = up_coord.local_to_global(local_dir);

        const Vec3 ex = -cross(camera_params_.up, dst_to_pos).normalize();
        const Vec3 ey = -cross(dst_to_pos, ex).normalize().normalize();

        camera_params_.dst = press_dst_ + panning_speed_ * camera_params_.distance * (real(dx) * ex + real(dy) * ey);

        update_gl_camera();
        editor_->on_change_camera();
        emit update_camera_panel();
    }
}

void Displayer::mouseReleaseEvent(QMouseEvent *event)
{
    gl_widget_->mouse_release(event);

    if(camera_state_ == CameraState::Move && event->button() == Qt::MouseButton::MiddleButton)
        camera_state_ = CameraState::Free;
    
    if(camera_state_ == CameraState::Rotate && event->button() == Qt::MouseButton::RightButton)
        camera_state_ = CameraState::Free;
}

void Displayer::wheelEvent(QWheelEvent *event)
{
    const real delta = real(0.05) * camera_params_.distance;
    camera_params_.distance -= delta * event->angleDelta().y() / real(120);
    camera_params_.distance = (std::max)(camera_params_.distance, real(0.01));

    update_gl_camera();
    editor_->on_change_camera();
    emit update_camera_panel();
}

Vec2 Displayer::pos_to_radian(const Vec3 &pos, const Vec3 &dst, const Vec3 &up)
{
    const tracer::Coord up_coord = tracer::Coord::from_z(up);
    const Vec3 local_dir = up_coord.global_to_local(pos - dst).normalize();
    const real vert_radian = tracer::local_angle::theta(local_dir);
    const real hori_radian = tracer::local_angle::phi(local_dir);
    return { hori_radian, vert_radian };
}

Vec3 Displayer::radian_to_pos(const Vec2 &radian, const Vec3 &dst, real distance, const Vec3 up)
{
    const auto up_coord = tracer::Coord::from_z(up);
    const Vec3 local_dir = {
        std::sin(radian.y) * std::cos(radian.x),
        std::sin(radian.y) * std::sin(radian.x),
        std::cos(radian.y)
    };
    const Vec3 global_dir = up_coord.local_to_global(local_dir);
    return dst + global_dir * distance;
}

void Displayer::update_gl_camera()
{
    const auto [width, height] = pixmap_size();
    const real aspect = real(width) / height;
    const Vec3 pos = radian_to_pos(
        camera_params_.radian, camera_params_.dst, camera_params_.distance, camera_params_.up);
    const Vec3 dst = camera_params_.dst;
    const Vec3 up  = camera_params_.up.normalize();
    const real fov = camera_params_.fov;

    QMatrix4x4 proj;
    proj.perspective(fov, aspect, real(0.1), real(1000));
    
    QMatrix4x4 view;
    view.lookAt(
        { pos.x, pos.y, pos.z },
        { dst.x, dst.y, dst.z },
        { up.x, up.y, up.z });

    gl_widget_->set_camera(proj * view, pos, dst - pos, up, fov);

    update();
}

AGZ_EDITOR_END
