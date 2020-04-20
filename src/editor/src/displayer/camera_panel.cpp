#include <QFormLayout>
#include <QLabel>

#include <agz/editor/displayer/camera_panel.h>
#include <agz/editor/imexport/asset_loader.h>
#include <agz/editor/imexport/asset_saver.h>
#include <agz/editor/ui/utility/collapsible.h>

AGZ_EDITOR_BEGIN

namespace
{
    Vec2 pos_to_radian(
        const Vec3 &pos, const Vec3 &dst, const Vec3 &up)
    {
        const tracer::Coord up_coord = tracer::Coord::from_z(up);
        const Vec3 local_dir = up_coord.global_to_local(pos - dst).normalize();
        const real vert_radian = tracer::local_angle::theta(local_dir);
        const real hori_radian = tracer::local_angle::phi(local_dir);
        return { hori_radian, vert_radian };
    }

    Vec3 radian_to_pos(
        const Vec2 &radian, const Vec3 &dst, real distance, const Vec3 up)
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
}

CameraParamsWidget::Params::Params()
{
    radian = pos_to_radian(position, look_at, up);
    distance = math::distance(position, look_at);
}

Vec3 CameraParamsWidget::Params::dir() const noexcept
{
    const auto up_coord = tracer::Coord::from_z(up);
    const Vec3 local_dir = {
        std::sin(radian.y) * std::cos(radian.x),
        std::sin(radian.y) * std::sin(radian.x),
        std::cos(radian.y)
    };
    const Vec3 global_dir = up_coord.local_to_global(local_dir);
    return -global_dir;
}

CameraParamsWidget::CameraParamsWidget()
{
    distance_ = new RealInput(this);
    radian_   = new Vec2Input(this);
    position_ = new Vec3Input(this);
    look_at_  = new Vec3Input(this);
    up_       = new Vec3Input(this);

    fov_            = new RealInput(this);
    lens_radius_    = new RealInput(this);
    focal_distance_ = new RealInput(this);

    QFormLayout *layout = new QFormLayout(this);
    layout->addRow("Distance",       distance_);
    layout->addRow("Radian",         radian_);
    layout->addRow("Position",       position_);
    layout->addRow("LookAt",         look_at_);
    layout->addRow("Up",             up_);
    layout->addRow("FOV",            fov_);
    layout->addRow("Lens Size",      lens_radius_);
    layout->addRow("Focal Distance", focal_distance_);

    connect(distance_, &RealInput::edit_value,
            this, &CameraParamsWidget::edit_distance);

    connect(radian_, &Vec2Input::edit_value,
            this, &CameraParamsWidget::edit_radian);

    connect(position_, &Vec3Input::edit_value,
            this, &CameraParamsWidget::edit_position);

    connect(look_at_, &Vec3Input::edit_value,
            this, &CameraParamsWidget::edit_look_at);

    connect(up_, &Vec3Input::edit_value,
            this, &CameraParamsWidget::edit_up);

    connect(fov_, &RealInput::edit_value,
            this, &CameraParamsWidget::edit_fov);

    connect(lens_radius_, &RealInput::edit_value,
            this, &CameraParamsWidget::edit_lens_radius);

    connect(focal_distance_, &RealInput::edit_value,
            this, &CameraParamsWidget::edit_focal_distance);

    update_ui_from_params();
}

const CameraParamsWidget::Params &CameraParamsWidget::get_params() const noexcept
{
    return params_;
}

void CameraParamsWidget::save_asset(AssetSaver &saver) const
{
    saver.write(params_);
}

void CameraParamsWidget::load_asset(AssetLoader &loader)
{
    params_ = loader.read<Params>();
    update_ui_from_params();
}

void CameraParamsWidget::set_distance(real new_distance)
{
    params_.distance = new_distance;

    const Vec3 new_pos = radian_to_pos(
        params_.radian, params_.look_at, params_.distance, params_.up);
    params_.position = new_pos;

    update_ui_from_params();
}

void CameraParamsWidget::set_radian(const Vec2 &new_radian)
{
    params_.radian = new_radian;

    const Vec3 new_pos = radian_to_pos(
        params_.radian, params_.look_at, params_.distance, params_.up);
    params_.position = new_pos;

    update_ui_from_params();
}

void CameraParamsWidget::set_look_at(const Vec3 &new_look_at)
{
    params_.look_at = new_look_at;

    const Vec3 new_pos = radian_to_pos(
        params_.radian, params_.look_at, params_.distance, params_.up);
    params_.position = new_pos;

    update_ui_from_params();
}

void CameraParamsWidget::copy_pos_from(const Params &params)
{
    params_.position = params.position;
    params_.look_at  = params.look_at;
    params_.radian   = params.radian;
    params_.distance = params.distance;
    params_.up       = params.up;

    update_ui_from_params();
}

void CameraParamsWidget::edit_distance(real new_dis)
{
    params_.distance = new_dis;

    const Vec3 new_pos = radian_to_pos(
        params_.radian, params_.look_at,
        params_.distance, params_.up);
    params_.position = new_pos;

    update_ui_from_params();
    emit edit_params();
}

void CameraParamsWidget::edit_radian(const Vec2 &new_radian)
{
    params_.radian = new_radian;

    const Vec3 new_pos = radian_to_pos(
        params_.radian, params_.look_at,
        params_.distance, params_.up);
    params_.position = new_pos;

    update_ui_from_params();
    emit edit_params();
}

void CameraParamsWidget::edit_position(const Vec3 &new_position)
{
    params_.position = new_position;
    
    const Vec2 new_rad = pos_to_radian(
        params_.position, params_.look_at, params_.up);
    params_.radian = new_rad;
    params_.distance = distance(params_.position, params_.look_at);

    update_ui_from_params();
    emit edit_params();
}

void CameraParamsWidget::edit_look_at(const Vec3 &look_at)
{
    params_.look_at = look_at;

    const Vec2 new_rad = pos_to_radian(
        params_.position, params_.look_at, params_.up);
    params_.radian = new_rad;
    params_.distance = distance(params_.position, params_.look_at);

    update_ui_from_params();
    emit edit_params();
}

void CameraParamsWidget::edit_up(const Vec3 &up)
{
    params_.up = up;

    const Vec2 new_rad = pos_to_radian(
        params_.position, params_.look_at, params_.up);
    params_.radian = new_rad;

    update_ui_from_params();
    emit edit_params();
}

void CameraParamsWidget::edit_fov(real fov)
{
    params_.fov_deg = fov;

    update_ui_from_params();
    emit edit_params();
}

void CameraParamsWidget::edit_lens_radius(real lens_radius)
{
    params_.lens_radius = lens_radius;

    update_ui_from_params();
    emit edit_params();
}

void CameraParamsWidget::edit_focal_distance(real focal_distance)
{
    params_.focal_distance = focal_distance;

    update_ui_from_params();
    emit edit_params();
}

void CameraParamsWidget::update_ui_from_params()
{
    distance_->set_value(params_.distance);
    radian_  ->set_value(params_.radian);
    position_->set_value(params_.position);
    look_at_ ->set_value(params_.look_at);
    up_      ->set_value(params_.up);

    fov_           ->set_value(params_.fov_deg);
    lens_radius_   ->set_value(params_.lens_radius);
    focal_distance_->set_value(params_.focal_distance);
}

CameraPanel::CameraPanel(QWidget *parent)
    : QWidget(parent)
{
    preview_ = new CameraParamsWidget;
    export_  = new CameraParamsWidget;

    set_preview_to_export_ = new QPushButton("Preview := Export", this);
    set_export_to_preview_ = new QPushButton("Export := Preview", this);
    editing_export_ = new QPushButton("Edit Export", this);

    export_width_ = new QLineEdit(this);
    export_height_ = new QLineEdit(this);
    export_framesize_validator_ = newBox<QIntValidator>();

    editing_export_->setCheckable(true);
    editing_export_->setChecked(false);
    export_->setDisabled(true);

    export_width_->setText("640");
    export_height_->setText("480");
    export_width_->setValidator(export_framesize_validator_.get());
    export_height_->setValidator(export_framesize_validator_.get());

    auto layout = new QVBoxLayout(this);
    layout->addWidget(preview_);

    auto export_sec = new Collapsible(this, "Export Camera");
    auto export_widget = new QWidget(export_sec);
    auto export_layout = new QFormLayout(export_widget);
    export_layout->addRow(set_preview_to_export_);
    export_layout->addRow(set_export_to_preview_);
    export_layout->addRow(editing_export_);
    export_layout->addRow("Framebuffer Width", export_width_);
    export_layout->addRow("Framebuffer Height", export_height_);
    export_layout->addRow(export_);

    export_sec->set_content_widget(export_widget);
    layout->addWidget(export_sec);

    export_sec->open();

    connect(set_export_to_preview_, &QPushButton::clicked,
            this, &CameraPanel::set_export_to_preview);

    connect(set_preview_to_export_, &QPushButton::clicked,
            this, &CameraPanel::set_preview_to_export);

    connect(editing_export_, &QPushButton::clicked,
            this, &CameraPanel::switch_editing_export);

    connect(export_width_, &QLineEdit::returnPressed,
            this, &CameraPanel::change_export_framebuffer_size);
    
    connect(export_height_, &QLineEdit::returnPressed,
            this, &CameraPanel::change_export_framebuffer_size);

    connect(preview_, &CameraParamsWidget::edit_params,
            [=]{ emit edit_params(); });
    
    connect(export_, &CameraParamsWidget::edit_params,
            [=]{ emit edit_params(); });

    update_display_params();
}

void CameraPanel::set_display_aspect(real aspect) noexcept
{
    display_window_aspect_ = aspect;
    update_display_params();
}

const CameraPanel::CameraParams &CameraPanel::get_display_params() const noexcept
{
    return display_params_;
}

const CameraPanel::CameraParams &CameraPanel::get_export_params() const noexcept
{
    return export_->get_params();
}

int CameraPanel::get_export_frame_width() const noexcept
{
    return export_width_->text().toInt();
}

int CameraPanel::get_export_frame_height() const noexcept
{
    return export_height_->text().toInt();
}

bool CameraPanel::is_export_frame_enabled() const noexcept
{
    return editing_export_->isChecked();
}

void CameraPanel::set_distance(real new_distance)
{
    if(editing_export_->isChecked())
        export_->set_distance(new_distance);
    else
        preview_->set_distance(new_distance);

    update_display_params();
}

void CameraPanel::set_radian(const Vec2 &new_radian)
{
    if(editing_export_->isChecked())
        export_->set_radian(new_radian);
    else
        preview_->set_radian(new_radian);

    update_display_params();
}

void CameraPanel::set_look_at(const Vec3 &new_look_at)
{
    if(editing_export_->isChecked())
        export_->set_look_at(new_look_at);
    else
        preview_->set_look_at(new_look_at);

    update_display_params();
}

void CameraPanel::save_asset(AssetSaver &saver) const
{
    preview_->save_asset(saver);
    export_->save_asset(saver);

    saver.write(int32_t(get_export_frame_width()));
    saver.write(int32_t(get_export_frame_height()));
}

void CameraPanel::load_asset(AssetLoader &loader)
{
    preview_->load_asset(loader);
    export_->load_asset(loader);

    export_width_->setText(QString::number(int(loader.read<int32_t>())));
    export_height_->setText(QString::number(int(loader.read<int32_t>())));

    update_display_params();
}

void CameraPanel::set_preview_to_export()
{
    preview_->copy_pos_from(export_->get_params());
    update_display_params();
    emit edit_params();
}

void CameraPanel::set_export_to_preview()
{
    export_->copy_pos_from(preview_->get_params());
    update_display_params();
    emit edit_params();
}

void CameraPanel::switch_editing_export()
{
    preview_->setDisabled(editing_export_->isChecked());
    export_->setDisabled(!editing_export_->isChecked());

    update_display_params();
    emit edit_params();
}

void CameraPanel::change_export_framebuffer_size()
{
    update_display_params();
    emit edit_params();
}

void CameraPanel::update_display_params()
{
    if(!editing_export_->isChecked())
    {
        display_params_ = preview_->get_params();
        return;
    }

    const real render_aspect = real(get_export_frame_width())
                             / get_export_frame_height();

    const auto export_params = export_->get_params();

    const real a = math::deg2rad(export_params.fov_deg);
    const real b1 = math::rad2deg(
        2 * std::atan(real(1.1) * std::tan(a / 2)));
    const real b2 = math::rad2deg(
        2 * std::atan(real(1.1) * render_aspect / display_window_aspect_
            * std::tan(a / 2)));

    const real min_display_fov = (std::max)(b1, b2);

    display_params_ = export_->get_params();
    display_params_.fov_deg = min_display_fov;
}

AGZ_EDITOR_END
