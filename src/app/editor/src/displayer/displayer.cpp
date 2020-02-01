#include <agz/editor/displayer/camera_panel.h>
#include <agz/editor/displayer/displayer.h>

AGZ_EDITOR_BEGIN

Displayer::Displayer(QWidget *parent)
    : QLabel(parent)
{
    setAlignment(Qt::AlignCenter);
    setScaledContents(true);
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
}

void Displayer::load_camera_from_config(const tracer::ConfigGroup &params)
{
    auto panel = get_camera_panel();

    const Vec3 pos = params.child_vec3("pos");
    panel->position_x->setValue(pos.x);
    panel->position_y->setValue(pos.y);
    panel->position_z->setValue(pos.z);

    const Vec3 dst = params.child_vec3("dst");
    panel->destination_x->setValue(dst.x);
    panel->destination_y->setValue(dst.y);
    panel->destination_z->setValue(dst.z);

    const Vec3 up = params.child_vec3("up");
    panel->up_x->setValue(up.x);
    panel->up_y->setValue(up.y);
    panel->up_z->setValue(up.z);

    const real fov = params.child_real("fov");
    panel->fov->setValue(fov);

    const real lens_radius = params.child_real_or("lens_radius", 0);
    panel->lens_radius->setValue(lens_radius);

    const real focal_distance = params.child_real_or("focal_distance", 1);
    panel->focal_distance->setValue(focal_distance);

    emit need_to_recreate_camera();
}

std::shared_ptr<tracer::Camera> Displayer::create_camera()
{
    const int film_width = size().width();
    const int film_height = size().height();
    const real film_aspect = static_cast<real>(film_width) / film_height;

    const DisplayerCameraParams params = get_camera_params();

    return tracer::create_thin_lens_camera(
        film_aspect,
        params.pos.map([](double v) { return real(v); }),
        params.dst.map([](double v) { return real(v); }),
        params.up.map([](double v) { return real(v); }),
        math::deg2rad(real(params.fov)),
        real(params.lens_radius),
        real(params.focal_distance));
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

    connect(camera_panel_->position_x, qOverload<double>(&QDoubleSpinBox::valueChanged),
        [=](double) { emit need_to_recreate_camera(); });
    connect(camera_panel_->position_y, qOverload<double>(&QDoubleSpinBox::valueChanged),
        [=](double) { emit need_to_recreate_camera(); });
    connect(camera_panel_->position_z, qOverload<double>(&QDoubleSpinBox::valueChanged),
        [=](double) { emit need_to_recreate_camera(); });

    connect(camera_panel_->destination_x, qOverload<double>(&QDoubleSpinBox::valueChanged),
        [=](double) { emit need_to_recreate_camera(); });
    connect(camera_panel_->destination_y, qOverload<double>(&QDoubleSpinBox::valueChanged),
        [=](double) { emit need_to_recreate_camera(); });
    connect(camera_panel_->destination_z, qOverload<double>(&QDoubleSpinBox::valueChanged),
        [=](double) { emit need_to_recreate_camera(); });

    connect(camera_panel_->up_x, qOverload<double>(&QDoubleSpinBox::valueChanged),
        [=](double) { emit need_to_recreate_camera(); });
    connect(camera_panel_->up_y, qOverload<double>(&QDoubleSpinBox::valueChanged),
        [=](double) { emit need_to_recreate_camera(); });
    connect(camera_panel_->up_z, qOverload<double>(&QDoubleSpinBox::valueChanged),
        [=](double) { emit need_to_recreate_camera(); });

    connect(camera_panel_->fov,            qOverload<double>(&QDoubleSpinBox::valueChanged),
        [=](double) { emit need_to_recreate_camera(); });
    connect(camera_panel_->lens_radius,    qOverload<double>(&QDoubleSpinBox::valueChanged),
        [=](double) { emit need_to_recreate_camera(); });
    connect(camera_panel_->focal_distance, qOverload<double>(&QDoubleSpinBox::valueChanged),
        [=](double) { emit need_to_recreate_camera(); });

    return camera_panel_;
}

void Displayer::resizeEvent(QResizeEvent *event)
{
    emit need_to_recreate_camera();
}

Displayer::DisplayerCameraParams Displayer::get_camera_params()
{
    auto panel = get_camera_panel();

    DisplayerCameraParams ret;
    
    ret.pos.x = panel->position_x->value();
    ret.pos.y = panel->position_y->value();
    ret.pos.z = panel->position_z->value();

    ret.dst.x = panel->destination_x->value();
    ret.dst.y = panel->destination_y->value();
    ret.dst.z = panel->destination_z->value();

    ret.up.x = panel->up_x->value();
    ret.up.y = panel->up_y->value();
    ret.up.z = panel->up_z->value();

    ret.fov            = panel->fov->value();
    ret.lens_radius    = panel->lens_radius->value();
    ret.focal_distance = panel->focal_distance->value();

    return ret;
}

AGZ_EDITOR_END
