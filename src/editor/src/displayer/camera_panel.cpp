#include <QFormLayout>
#include <QLabel>

#include <agz/editor/displayer/camera_panel.h>
#include <agz/editor/imexport/asset_loader.h>
#include <agz/editor/imexport/asset_saver.h>
#include <agz/editor/ui/utility/collapsible.h>

AGZ_EDITOR_BEGIN

Vec3 CameraPanel::CameraParams::dir() const noexcept
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

CameraPanel::CameraPanel(QWidget *parent)
    : QWidget(parent)
{
    init_ui();

    preview_.distance      ->set_value(4);
    preview_.radian        ->set_value({ 0, PI_r / 2 });
    preview_.position      ->set_value(radian_to_pos({ 0, PI_r / 2 }, {}, 4, { 0, 0, 1 }));
    preview_.look_at       ->set_value({});
    preview_.up            ->set_value({ 0, 0, 1 });
    preview_.fov           ->set_value(60);
    preview_.lens_radius   ->set_value(0);
    preview_.focal_distance->set_value(1);
    
    render_.distance      ->set_value(4);
    render_.radian        ->set_value({ 0, PI_r / 2 });
    render_.position      ->set_value(radian_to_pos({ 0, PI_r / 2 }, {}, 4, { 0, 0, 1 }));
    render_.look_at       ->set_value({});
    render_.up            ->set_value({ 0, 0, 1 });
    render_.fov           ->set_value(60);
    render_.lens_radius   ->set_value(0);
    render_.focal_distance->set_value(1);

    fetch_params_from_ui();

    connect(preview_.distance, &RealInput::edit_value, this, &CameraPanel::edit_distance);
    connect(preview_.radian,   &Vec2Input::edit_value, this, &CameraPanel::edit_radian);
    connect(preview_.position, &Vec3Input::edit_value, this, &CameraPanel::edit_position);
    connect(preview_.look_at,  &Vec3Input::edit_value, this, &CameraPanel::edit_look_at);
    connect(preview_.up,       &Vec3Input::edit_value, this, &CameraPanel::edit_up);

    connect(preview_.fov,            &RealInput::edit_value, this, &CameraPanel::edit_fov);
    connect(preview_.lens_radius,    &RealInput::edit_value, this, &CameraPanel::edit_lens_radius);
    connect(preview_.focal_distance, &RealInput::edit_value, this, &CameraPanel::edit_focal_distance);

    connect(render_.display_render_camera, &QPushButton::clicked, this, &CameraPanel::click_set_render);
    connect(render_.bind_render_camera,    &QPushButton::clicked, this, &CameraPanel::click_bind_render);

    connect(render_.render_width,  &QLineEdit::returnPressed, [=] { edit_render_framesize(); });
    connect(render_.render_height, &QLineEdit::returnPressed, [=] { edit_render_framesize(); });
}

void CameraPanel::set_preview_aspect(real aspect) noexcept
{
    preview_aspect_ = aspect;
}

const CameraPanel::CameraParams &CameraPanel::get_preview_params() const noexcept
{
    return preview_params_;
}

const CameraPanel::CameraParams &CameraPanel::get_render_params() const noexcept
{
    return render_params_;
}

int CameraPanel::get_render_frame_width() const noexcept
{
    return render_width_;
}

int CameraPanel::get_render_frame_height() const noexcept
{
    return render_height_;
}

bool CameraPanel::is_render_frame_enabled() const noexcept
{
    return enable_render_frame_;
}

void CameraPanel::set_distance(real new_distance)
{
    if(!render_.bind_render_camera->isChecked())
        enable_render_frame_ = false;

    const Vec3 new_pos = radian_to_pos(
        preview_params_.radian, preview_params_.look_at, new_distance, preview_params_.up);

    preview_.distance->set_value(new_distance);
    preview_.position->set_value(new_pos);

    if(render_.bind_render_camera->isChecked())
    {
        render_.distance->set_value(new_distance);
        render_.position->set_value(new_pos);
    }

    fetch_params_from_ui();
}

void CameraPanel::set_radian(const Vec2 &new_radian)
{
    if(!render_.bind_render_camera->isChecked())
        enable_render_frame_ = false;

    const Vec3 new_pos = radian_to_pos(
        new_radian, preview_params_.look_at, preview_params_.distance, preview_params_.up);

    preview_.radian  ->set_value(new_radian);
    preview_.position->set_value(new_pos);

    if(render_.bind_render_camera->isChecked())
    {
        render_.radian->set_value(new_radian);
        render_.position->set_value(new_pos);
    }

    fetch_params_from_ui();
}

void CameraPanel::set_look_at(const Vec3 &new_look_at)
{
    if(!render_.bind_render_camera->isChecked())
        enable_render_frame_ = false;

    const Vec3 new_pos = radian_to_pos(
        preview_params_.radian, new_look_at, preview_params_.distance, preview_params_.up);

    preview_.look_at ->set_value(new_look_at);
    preview_.position->set_value(new_pos);

    if(render_.bind_render_camera->isChecked())
    {
        render_.look_at->set_value(new_look_at);
        render_.position->set_value(new_pos);
    }

    fetch_params_from_ui();
}

void CameraPanel::save_asset(AssetSaver &saver) const
{
    saver.write(preview_params_);
    saver.write(render_params_);
    saver.write(int32_t(render_width_));
    saver.write(int32_t(render_height_));
}

void CameraPanel::load_asset(AssetLoader &loader)
{
    preview_params_ = loader.read<CameraParams>();
    render_params_  = loader.read<CameraParams>();

    render_width_  = int(loader.read<int32_t>());
    render_height_ = int(loader.read<int32_t>());
    render_.render_width ->setText(QString::number(render_width_));
    render_.render_height->setText(QString::number(render_height_));

    preview_.distance      ->set_value(preview_params_.distance);
    preview_.radian        ->set_value(preview_params_.radian);
    preview_.position      ->set_value(preview_params_.position);
    preview_.look_at       ->set_value(preview_params_.look_at);
    preview_.up            ->set_value(preview_params_.up);
    preview_.fov           ->set_value(preview_params_.fov_deg);
    preview_.lens_radius   ->set_value(preview_params_.lens_radius);
    preview_.focal_distance->set_value(preview_params_.focal_distance);

    render_.distance      ->set_value(render_params_.distance);
    render_.radian        ->set_value(render_params_.radian);
    render_.position      ->set_value(render_params_.position);
    render_.look_at       ->set_value(render_params_.look_at);
    render_.up            ->set_value(render_params_.up);
    render_.fov           ->set_value(render_params_.fov_deg);
    render_.lens_radius   ->set_value(render_params_.lens_radius);
    render_.focal_distance->set_value(render_params_.focal_distance);
}

void CameraPanel::edit_distance(real new_distance)
{
    const Vec3 new_pos = radian_to_pos(preview_params_.radian, preview_params_.look_at, new_distance, preview_params_.up);
    preview_.position->set_value(new_pos);

    if(render_.bind_render_camera->isChecked())
    {
        render_.distance->set_value(new_distance);
        render_.position->set_value(new_pos);
    }
    else
        enable_render_frame_ = false;

    fetch_params_from_ui();
    emit edit_params();
}

void CameraPanel::edit_radian(const Vec2 &new_radian)
{
    const Vec3 new_pos = radian_to_pos(new_radian, preview_params_.look_at, preview_params_.distance, preview_params_.up);
    preview_.position->set_value(new_pos);

    if(render_.bind_render_camera->isChecked())
    {
        render_.radian->set_value(new_radian);
        render_.position->set_value(new_pos);
    }
    else
        enable_render_frame_ = false;

    fetch_params_from_ui();
    emit edit_params();
}

void CameraPanel::edit_position(const Vec3 &new_position)
{
    const Vec2 new_rad = pos_to_radian(new_position, preview_params_.look_at, preview_params_.up);
    const real new_dis = distance(new_position, preview_params_.look_at);

    preview_.radian  ->set_value(new_rad);
    preview_.distance->set_value(new_dis);

    if(render_.bind_render_camera->isChecked())
    {
        render_.position->set_value(new_position);
        render_.radian->set_value(new_rad);
        render_.distance->set_value(new_dis);
    }
    else
        enable_render_frame_ = false;

    fetch_params_from_ui();
    emit edit_params();
}

void CameraPanel::edit_look_at(const Vec3 &new_look_at)
{
    const Vec3 new_pos = radian_to_pos(
        preview_params_.radian, new_look_at, preview_params_.distance, preview_params_.up);
    preview_.position->set_value(new_pos);

    if(render_.bind_render_camera->isChecked())
    {
        render_.look_at->set_value(new_look_at);
        render_.position->set_value(new_pos);
    }
    else
        enable_render_frame_ = false;

    fetch_params_from_ui();
    emit edit_params();
}

void CameraPanel::edit_up(const Vec3 &up)
{
    if(render_.bind_render_camera->isChecked())
        render_.up->set_value(up);
    else
        enable_render_frame_ = false;

    fetch_params_from_ui();
    emit edit_params();
}

void CameraPanel::edit_fov(real new_fov_deg)
{
    if(render_.bind_render_camera->isChecked())
    {
        const real min_fov_deg = min_preview_fov_deg();
        if(new_fov_deg < min_fov_deg)
        {
            new_fov_deg = min_fov_deg;
            preview_.fov->set_value(new_fov_deg);
        }
    }

    fetch_params_from_ui();
    emit edit_params();
}

void CameraPanel::edit_lens_radius(real new_lens_radius)
{
    fetch_params_from_ui();
    emit edit_params();
}

void CameraPanel::edit_focal_distance(real new_focal_distance)
{
    fetch_params_from_ui();
    emit edit_params();
}

void CameraPanel::edit_render_framesize()
{
    render_width_  = render_.render_width ->text().toInt();
    render_height_ = render_.render_height->text().toInt();

    if(render_.bind_render_camera->isChecked())
    {
        preview_.fov->set_value((std::max)(preview_.fov->get_value(), min_preview_fov_deg()));
        fetch_params_from_ui();
    }
    emit edit_params();
}

void CameraPanel::click_set_render()
{
    enable_render_frame_ = true;

    preview_.distance->set_value(render_.distance->get_value());
    preview_.radian  ->set_value(render_.radian  ->get_value());
    preview_.position->set_value(render_.position->get_value());
    preview_.look_at ->set_value(render_.look_at ->get_value());
    preview_.up      ->set_value(render_.up      ->get_value());

    preview_.fov->set_value((std::max)(preview_.fov->get_value(), min_preview_fov_deg()));

    fetch_params_from_ui();
    emit edit_params();
}

void CameraPanel::click_bind_render()
{
    const bool disable_render_editor = render_.bind_render_camera->isChecked();
    render_.distance->setDisabled(disable_render_editor);
    render_.radian  ->setDisabled(disable_render_editor);
    render_.position->setDisabled(disable_render_editor);
    render_.look_at ->setDisabled(disable_render_editor);
    render_.up      ->setDisabled(disable_render_editor);
    render_.position->setDisabled(disable_render_editor);

    if(!render_.bind_render_camera->isChecked())
        return;

    enable_render_frame_ = true;

    preview_.fov->set_value((std::max)(preview_.fov->get_value(), min_preview_fov_deg()));

    render_.distance->set_value(preview_.distance->get_value());
    render_.radian  ->set_value(preview_.radian  ->get_value());
    render_.position->set_value(preview_.position->get_value());
    render_.look_at ->set_value(preview_.look_at ->get_value());
    render_.up      ->set_value(preview_.up      ->get_value());

    fetch_params_from_ui();
    emit edit_params();
}

Vec2 CameraPanel::pos_to_radian(const Vec3 &pos, const Vec3 &dst, const Vec3 &up)
{
    const tracer::Coord up_coord = tracer::Coord::from_z(up);
    const Vec3 local_dir = up_coord.global_to_local(pos - dst).normalize();
    const real vert_radian = tracer::local_angle::theta(local_dir);
    const real hori_radian = tracer::local_angle::phi(local_dir);
    return { hori_radian, vert_radian };
}

Vec3 CameraPanel::radian_to_pos(const Vec2 &radian, const Vec3 &dst, real distance, const Vec3 up)
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

void CameraPanel::init_ui()
{
    // preview section

    QWidget     *preview_widget = new QWidget(this);
    QFormLayout *preview_layout = new QFormLayout(preview_widget);

    preview_.distance = new RealInput(preview_widget);
    preview_.radian   = new Vec2Input(preview_widget);
    preview_.position = new Vec3Input(preview_widget);
    preview_.look_at  = new Vec3Input(preview_widget);
    preview_.up       = new Vec3Input(preview_widget);

    preview_.fov            = new RealInput(preview_widget);
    preview_.lens_radius    = new RealInput(preview_widget);
    preview_.focal_distance = new RealInput(preview_widget);

    preview_layout->addRow(new QLabel("Distance"),       preview_.distance);
    preview_layout->addRow(new QLabel("Radian"),         preview_.radian);
    preview_layout->addRow(new QLabel("Position"),       preview_.position);
    preview_layout->addRow(new QLabel("LookAt"),         preview_.look_at);
    preview_layout->addRow(new QLabel("Up"),             preview_.up);
    preview_layout->addRow(new QLabel("FOV"),            preview_.fov);
    preview_layout->addRow(new QLabel("Lens Size"),      preview_.lens_radius);
    preview_layout->addRow(new QLabel("Focal Distance"), preview_.focal_distance);

    preview_widget->setContentsMargins(0, 0, 0, 0);
    preview_layout->setContentsMargins(0, 0, 0, 0);

    Collapsible *preview_sec = new Collapsible(this, "Preview");
    preview_sec->set_content_widget(preview_widget);
    preview_sec->open();

    // render section

    QWidget     *render_widget = new QWidget(this);
    QGridLayout *render_layout = new QGridLayout(render_widget);

    render_.distance = new RealInput(render_widget);
    render_.radian   = new Vec2Input(render_widget);
    render_.position = new Vec3Input(render_widget);
    render_.look_at  = new Vec3Input(render_widget);
    render_.up       = new Vec3Input(render_widget);

    render_.fov            = new RealInput(render_widget);
    render_.lens_radius    = new RealInput(render_widget);
    render_.focal_distance = new RealInput(render_widget);

    render_.display_render_camera = new QPushButton("Preview");
    render_.bind_render_camera    = new QPushButton("Bind");

    render_.display_render_camera->setToolTip("preview rendering camera");
    render_.bind_render_camera   ->setToolTip("bind rendering camera to preview window");

    render_.bind_render_camera->setCheckable(true);
    render_.bind_render_camera->setChecked(false);

    render_.render_width  = new QLineEdit(this);
    render_.render_height = new QLineEdit(this);

    render_framesize_validator_ = std::make_unique<QIntValidator>(this);
    render_framesize_validator_->setRange(1, 999999);

    render_.render_width ->setValidator(render_framesize_validator_.get());
    render_.render_height->setValidator(render_framesize_validator_.get());
    render_.render_width ->setText(QString::number(render_width_));
    render_.render_height->setText(QString::number(render_height_));
    
    int render_row = 0;

    render_layout->addWidget(render_.display_render_camera,   render_row, 0, 1, 2);
    render_layout->addWidget(render_.bind_render_camera,    ++render_row, 0, 1, 2);

    render_layout->addWidget(new QLabel("Frame Width"), ++render_row, 0);
    render_layout->addWidget(render_.render_width, render_row, 1);

    render_layout->addWidget(new QLabel("Frame Height"), ++render_row, 0);
    render_layout->addWidget(render_.render_height, render_row, 1);

    render_layout->addWidget(new QLabel("Distance"), ++render_row, 0);
    render_layout->addWidget(render_.distance,         render_row, 1);

    render_layout->addWidget(new QLabel("Radian"), ++render_row, 0);
    render_layout->addWidget(render_.radian,         render_row, 1);

    render_layout->addWidget(new QLabel("Position"), ++render_row, 0);
    render_layout->addWidget(render_.position,         render_row, 1);

    render_layout->addWidget(new QLabel("LookAt"), ++render_row, 0);
    render_layout->addWidget(render_.look_at,        render_row, 1);

    render_layout->addWidget(new QLabel("Up"), ++render_row, 0);
    render_layout->addWidget(render_.up,         render_row, 1);

    render_layout->addWidget(new QLabel("FOV"), ++render_row, 0);
    render_layout->addWidget(render_.fov,         render_row, 1);

    render_layout->addWidget(new QLabel("Lens Size"), ++render_row, 0);
    render_layout->addWidget(render_.lens_radius,       render_row, 1);

    render_layout->addWidget(new QLabel("Focal Distance"), ++render_row, 0);
    render_layout->addWidget(render_.focal_distance,         render_row, 1);

    render_widget->setContentsMargins(0, 0, 0, 0);
    render_layout->setContentsMargins(0, 0, 0, 0);

    Collapsible *render_sec = new Collapsible(this, "Render");
    render_sec->set_content_widget(render_widget);
    
    // panel layout

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(preview_sec);
    layout->addWidget(render_sec);

    setContentsMargins(0, 0, 0, 0);
    layout->setContentsMargins(0, 0, 0, 0);
}

void CameraPanel::fetch_params_from_ui()
{
    preview_params_.distance       = preview_.distance      ->get_value();
    preview_params_.radian         = preview_.radian        ->get_value();
    preview_params_.position       = preview_.position      ->get_value();
    preview_params_.look_at        = preview_.look_at       ->get_value();
    preview_params_.up             = preview_.up            ->get_value();
    preview_params_.fov_deg        = preview_.fov           ->get_value();
    preview_params_.lens_radius    = preview_.lens_radius   ->get_value();
    preview_params_.focal_distance = preview_.focal_distance->get_value();
    
    render_params_.distance       = render_.distance      ->get_value();
    render_params_.radian         = render_.radian        ->get_value();
    render_params_.position       = render_.position      ->get_value();
    render_params_.look_at        = render_.look_at       ->get_value();
    render_params_.up             = render_.up            ->get_value();
    render_params_.fov_deg        = render_.fov           ->get_value();
    render_params_.lens_radius    = render_.lens_radius   ->get_value();
    render_params_.focal_distance = render_.focal_distance->get_value();
}

real CameraPanel::min_preview_fov_deg()
{
    const real render_aspect = real(render_width_) / render_height_;

    const real a = math::deg2rad(render_params_.fov_deg);
    const real b1 = math::rad2deg(
        2 * std::atan(real(1.1) * std::tan(a / 2)));
    const real b2 = math::rad2deg(
        2 * std::atan(real(1.1) * render_aspect / preview_aspect_ * std::tan(a / 2)));

    return (std::max)(b1, b2);
}

AGZ_EDITOR_END
