#include <QMouseEvent>
#include <QWheelEvent>

#include <agz/editor/displayer/camera_panel.h>
#include <agz/editor/displayer/displayer.h>
#include <agz/editor/editor.h>

AGZ_EDITOR_BEGIN

Displayer::Displayer(QWidget *parent, Editor *editor)
    : QLabel(parent), editor_(editor)
{
    setAlignment(Qt::AlignCenter);
    setScaledContents(true);
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
}

void Displayer::load_camera_from_config(const tracer::ConfigGroup &params)
{
    const Vec3 pos = params.child_vec3("pos");
    
    camera_params_.dst = params.child_vec3("dst");
    
    camera_params_.up = params.child_vec3("up");

    camera_params_.distance = distance(pos, camera_params_.dst);

    camera_params_.radian = pos_to_radian(pos, camera_params_.dst, camera_params_.up);
    
    camera_params_.fov = params.child_real("fov");
    
    camera_params_.lens_radius = params.child_real_or("lens_radius", 0);
    
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

    camera_panel_->position_x->setValue(5);
    camera_panel_->position_y->setValue(0);
    camera_panel_->position_z->setValue(0);

    camera_panel_->destination_x->setValue(0);
    camera_panel_->destination_y->setValue(0);
    camera_panel_->destination_z->setValue(0);

    camera_panel_->up_x->setValue(0);
    camera_panel_->up_y->setValue(0);
    camera_panel_->up_z->setValue(1);

    camera_panel_->fov           ->setValue(60);
    camera_panel_->lens_radius   ->setValue(0);
    camera_panel_->focal_distance->setValue(1);

    // do something while keeping camera_params_.pos unchanged
    auto keep_pos = [this](const std::function<void()> &f)
    {
        const Vec3 old_pos = radian_to_pos(
            camera_params_.radian, camera_params_.dst,
            camera_params_.distance, camera_params_.up);
        f();
        camera_params_.radian = pos_to_radian(old_pos, camera_params_.dst, camera_params_.up);
    };

    connect(camera_panel_->position_x, qOverload<double>(&QDoubleSpinBox::valueChanged),
        [=](double value)
    {
        const Vec3 old_pos = radian_to_pos(
            camera_params_.radian, camera_params_.dst,
            camera_params_.distance, camera_params_.up);
        const Vec3 new_pos = Vec3(real(value), old_pos.y, old_pos.z);
        camera_params_.radian = pos_to_radian(new_pos, camera_params_.dst, camera_params_.up);
        emit need_to_recreate_camera();
    });

    connect(camera_panel_->position_y, qOverload<double>(&QDoubleSpinBox::valueChanged),
        [=](double value)
    {
        const Vec3 old_pos = radian_to_pos(
            camera_params_.radian, camera_params_.dst,
            camera_params_.distance, camera_params_.up);
        const Vec3 new_pos = Vec3(old_pos.x, real(value), old_pos.z);
        camera_params_.radian = pos_to_radian(new_pos, camera_params_.dst, camera_params_.up);
        emit need_to_recreate_camera();
    });

    connect(camera_panel_->position_z, qOverload<double>(&QDoubleSpinBox::valueChanged),
        [=](double value)
    {
        const Vec3 old_pos = radian_to_pos(
            camera_params_.radian, camera_params_.dst,
            camera_params_.distance, camera_params_.up);
        const Vec3 new_pos = Vec3(old_pos.x, old_pos.y, real(value));
        camera_params_.radian = pos_to_radian(new_pos, camera_params_.dst, camera_params_.up);
        emit need_to_recreate_camera();
    });
    
    connect(camera_panel_->destination_x, qOverload<double>(&QDoubleSpinBox::valueChanged),
        [=](double value)
    {
        keep_pos([=] { camera_params_.dst.x = real(value); });
        emit need_to_recreate_camera();
    });

    connect(camera_panel_->destination_y, qOverload<double>(&QDoubleSpinBox::valueChanged),
        [=](double value)
    {
        keep_pos([=] { camera_params_.dst.y = real(value); });
        emit need_to_recreate_camera();
    });

    connect(camera_panel_->destination_z, qOverload<double>(&QDoubleSpinBox::valueChanged),
        [=](double value)
    {
        keep_pos([=] { camera_params_.dst.z = real(value); });
        emit need_to_recreate_camera();
    });
    
    connect(camera_panel_->up_x, qOverload<double>(&QDoubleSpinBox::valueChanged),
        [=](double value)
    {
        keep_pos([=] { camera_params_.up.x = real(value); });
        emit need_to_recreate_camera();
    });

    connect(camera_panel_->up_y, qOverload<double>(&QDoubleSpinBox::valueChanged),
        [=](double value)
    {
        keep_pos([=] { camera_params_.up.y = real(value); });
        emit need_to_recreate_camera();
    });

    connect(camera_panel_->up_z, qOverload<double>(&QDoubleSpinBox::valueChanged),
        [=](double value)
    {
        keep_pos([=] { camera_params_.up.z = real(value); });
        emit need_to_recreate_camera();
    });
    
    connect(camera_panel_->fov, qOverload<double>(&QDoubleSpinBox::valueChanged),
        [=](double value)
    {
        camera_params_.fov = real(value);
        emit need_to_recreate_camera();
    });

    connect(camera_panel_->lens_radius, qOverload<double>(&QDoubleSpinBox::valueChanged),
        [=](double value)
    {
        camera_params_.lens_radius = real(value);
        emit need_to_recreate_camera();
    });

    connect(camera_panel_->focal_distance, qOverload<double>(&QDoubleSpinBox::valueChanged),
        [=](double value)
    {
        camera_params_.focal_distance = real(value);
        emit need_to_recreate_camera();
    });

    connect(this, &Displayer::update_camera_panel, [=]
    {
        auto panel = get_camera_panel();

        const Vec3 pos = radian_to_pos(
            camera_params_.radian, camera_params_.dst,
            camera_params_.distance, camera_params_.up);

        panel->position_x->blockSignals(true);
        panel->position_y->blockSignals(true);
        panel->position_z->blockSignals(true);
        panel->position_x->setValue(pos.x);
        panel->position_y->setValue(pos.y);
        panel->position_z->setValue(pos.z);
        panel->position_x->blockSignals(false);
        panel->position_y->blockSignals(false);
        panel->position_z->blockSignals(false);

        panel->destination_x->blockSignals(true);
        panel->destination_y->blockSignals(true);
        panel->destination_z->blockSignals(true);
        panel->destination_x->setValue(camera_params_.dst.x);
        panel->destination_y->setValue(camera_params_.dst.y);
        panel->destination_z->setValue(camera_params_.dst.z);
        panel->destination_x->blockSignals(false);
        panel->destination_y->blockSignals(false);
        panel->destination_z->blockSignals(false);

        panel->up_x->blockSignals(true);
        panel->up_y->blockSignals(true);
        panel->up_z->blockSignals(true);
        panel->up_x->setValue(camera_params_.up.x);
        panel->up_y->setValue(camera_params_.up.y);
        panel->up_z->setValue(camera_params_.up.z);
        panel->up_x->blockSignals(false);
        panel->up_y->blockSignals(false);
        panel->up_z->blockSignals(false);

        panel->fov           ->blockSignals(true);
        panel->lens_radius   ->blockSignals(true);
        panel->focal_distance->blockSignals(true);
        panel->fov           ->setValue(camera_params_.fov);
        panel->lens_radius   ->setValue(camera_params_.lens_radius);
        panel->focal_distance->setValue(camera_params_.focal_distance);
        panel->fov           ->blockSignals(false);
        panel->lens_radius   ->blockSignals(false);
        panel->focal_distance->blockSignals(false);
    });

    emit update_camera_panel();

    return camera_panel_;
}

void Displayer::set_camera_rotate_speed(real speed)
{
    rotate_speed_ = speed;
}

void Displayer::leaveEvent(QEvent *event)
{
    camera_state_ = CameraState::Free;
}

void Displayer::resizeEvent(QResizeEvent *event)
{
    emit need_to_recreate_camera();
}

void Displayer::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::MouseButton::RightButton)
    {
        camera_state_ = CameraState::Rotate;
        press_coord_ = { event->x(), event->y() };
        press_radian_ = camera_params_.radian;
        press_dst_ = camera_params_.dst;
        return;
    }

    if(event->button() == Qt::MouseButton::MiddleButton)
    {
        camera_state_ = CameraState::Move;
        press_coord_ = { event->x(), event->y() };
        press_radian_ = camera_params_.radian;
        press_dst_ = camera_params_.dst;
    }
}

void Displayer::mouseMoveEvent(QMouseEvent *event)
{
    if(camera_state_ == CameraState::Rotate)
    {
        const int dx = event->x() - press_coord_.x;
        const int dy = event->y() - press_coord_.y;

        camera_params_.radian.x = press_radian_.x - rotate_speed_ * real(dx);
        camera_params_.radian.y = math::clamp<real>(
            press_radian_.y - rotate_speed_ * dy, real(0.001), PI_r - real(0.001));

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
        editor_->on_change_camera();
        emit update_camera_panel();
    }
}

void Displayer::mouseReleaseEvent(QMouseEvent *event)
{
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

AGZ_EDITOR_END
