#include <QGridLayout>
#include <QLabel>

#include <agz/editor/imexport/asset_loader.h>
#include <agz/editor/imexport/asset_saver.h>
#include <agz/editor/renderer/export/export_renderer_sppm.h>

AGZ_EDITOR_BEGIN

ExportRendererSPPM::ExportRendererSPPM(QWidget *parent)
    : ExportRendererWidget(parent)
{
    QGridLayout *layout = new QGridLayout(this);
    int row = 0;

    // worker count

    worker_count_ = new QSpinBox(this);
    worker_count_->setRange(
        (std::numeric_limits<int>::lowest)(),
        (std::numeric_limits<int>::max)());
    worker_count_->setValue(-1);

    layout->addWidget(new QLabel("Thread Count"), row, 0, 1, 1);
    layout->addWidget(worker_count_, row, 1, 1, 2);

    // task grid size

    forward_task_grid_size_ = new QSpinBox(this);
    forward_task_grid_size_->setRange(1, std::numeric_limits<int>::max());
    forward_task_grid_size_->setValue(128);

    layout->addWidget(new QLabel("Task Size"), ++row, 0, 1, 1);
    layout->addWidget(forward_task_grid_size_, row, 1, 1, 2);

    // forward max depth

    auto forward_max_depth_display = new QLabel(this);

    forward_max_depth_ = new QSlider(this);
    forward_max_depth_->setOrientation(Qt::Horizontal);
    forward_max_depth_->setRange(1, 20);

    connect(forward_max_depth_, &QSlider::valueChanged, [=](int new_val)
    {
        forward_max_depth_display->setText(QString::number(new_val));
    });

    forward_max_depth_->setValue(8);

    layout->addWidget(new QLabel("Max Camera Depth"), ++row, 0, 1, 1);
    layout->addWidget(forward_max_depth_display, row, 1, 1, 1);
    layout->addWidget(forward_max_depth_, row, 2, 1, 1);

    // initial radius

    init_radius_ = new RealInput(this);
    init_radius_->set_value(-1);

    layout->addWidget(new QLabel("Initial Radius"), ++row, 0, 1, 1);
    layout->addWidget(init_radius_, row, 1, 1, 2);

    // iteration count

    iteration_count_ = new QSpinBox(this);
    iteration_count_->setRange(1, std::numeric_limits<int>::max());
    iteration_count_->setValue(1000);

    layout->addWidget(new QLabel("Iteration Count"), ++row, 0, 1, 1);
    layout->addWidget(iteration_count_, row, 1, 1, 2);

    // photons per iteration

    photons_per_iteration_ = new QSpinBox(this);
    photons_per_iteration_->setRange(1, std::numeric_limits<int>::max());
    photons_per_iteration_->setValue(100000);

    layout->addWidget(new QLabel("Photons per Iteration"), ++row, 0, 1, 1);
    layout->addWidget(photons_per_iteration_, row, 1, 1, 2);

    // photon min/max depth

    auto photon_min_depth_display = new QLabel(this);
    auto photon_max_depth_display = new QLabel(this);

    photon_min_depth_ = new QSlider(this);
    photon_max_depth_ = new QSlider(this);
    photon_min_depth_->setOrientation(Qt::Horizontal);
    photon_max_depth_->setOrientation(Qt::Horizontal);
    photon_min_depth_->setRange(1, 20);
    photon_max_depth_->setRange(1, 20);

    connect(photon_min_depth_, &QSlider::valueChanged, [=](int new_value)
    {
        photon_min_depth_display->setText(QString::number(new_value));
    });

    connect(photon_max_depth_, &QSlider::valueChanged, [=](int new_value)
    {
        photon_max_depth_display->setText(QString::number(new_value));
    });

    photon_min_depth_->setValue(5);
    photon_max_depth_->setValue(10);

    connect(photon_min_depth_, &QSlider::valueChanged, [=](int new_val)
    {
        if(new_val > photon_max_depth_->value())
        {
            photon_max_depth_->blockSignals(true);
            photon_max_depth_->setValue(new_val);
            photon_max_depth_->blockSignals(false);

            photon_max_depth_display->setText(QString::number(new_val));
        }
    });

    connect(photon_max_depth_, &QSlider::valueChanged, [=](int new_val)
    {
        if(new_val < photon_min_depth_->value())
        {
            photon_min_depth_->blockSignals(true);
            photon_min_depth_->setValue(new_val);
            photon_min_depth_->blockSignals(false);

            photon_min_depth_display->setText(QString::number(new_val));
        }
    });

    layout->addWidget(new QLabel("Min Photon Depth"), ++row, 0, 1, 1);
    layout->addWidget(photon_min_depth_display, row, 1, 1, 1);
    layout->addWidget(photon_min_depth_, row, 2, 1, 1);

    layout->addWidget(new QLabel("Max Photon Depth"), ++row, 0, 1, 1);
    layout->addWidget(photon_max_depth_display, row, 1, 1, 1);
    layout->addWidget(photon_max_depth_, row, 2, 1, 1);

    // photon cont prob

    auto photon_cont_prob_display = new QLabel(this);
    photon_cont_prob_display->setText("0.9");

    photon_cont_prob_ = new QSlider(this);
    photon_cont_prob_->setOrientation(Qt::Horizontal);
    photon_cont_prob_->setRange(1, 10);

    connect(photon_cont_prob_, &QSlider::valueChanged, [=](int new_val)
    {
        photon_cont_prob_display->setText(QString::number(new_val / real(10)));
    });

    photon_cont_prob_->setValue(9);

    layout->addWidget(new QLabel("Photon Cont Prob"), ++row, 0, 1, 1);
    layout->addWidget(photon_cont_prob_display, row, 1, 1, 1);
    layout->addWidget(photon_cont_prob_, row, 2, 1, 1);

    // alpha

    alpha_ = new RealInput(this);
    alpha_->set_value(real(2) / 3);

    layout->addWidget(new QLabel("Update Alpha"), ++row, 0, 1, 1);
    layout->addWidget(alpha_, row, 1, 1, 2);

    // grid res

    grid_res_ = new QSpinBox(this);
    grid_res_->setRange(1, std::numeric_limits<int>::max());
    grid_res_->setValue(64);

    layout->addWidget(new QLabel("Accel Grid Resolution"), ++row, 0, 1, 1);
    layout->addWidget(grid_res_, row, 1, 1, 2);

    setContentsMargins(0, 0, 0, 0);
    layout->setContentsMargins(0, 0, 0, 0);
}

RC<tracer::ConfigGroup> ExportRendererSPPM::to_config() const
{
    auto grp = newRC<tracer::ConfigGroup>();

    grp->insert_str("type", "sppm");

    grp->insert_int("worker_count", worker_count_->value());
    grp->insert_int("task_grid_size", forward_task_grid_size_->value());
    grp->insert_int("forward_max_depth", forward_max_depth_->value());

    grp->insert_real("init_radius", init_radius_->get_value());

    grp->insert_int("iteration_count", iteration_count_->value());
    grp->insert_int("photons_per_iteration", photons_per_iteration_->value());

    grp->insert_int("photon_min_depth", photon_min_depth_->value());
    grp->insert_int("photon_max_depth", photon_max_depth_->value());
    grp->insert_real("photon_cont_prob", photon_cont_prob_->value() / real(10));

    grp->insert_real("alpha", alpha_->get_value());
    grp->insert_int("grid_res", grid_res_->value());

    return grp;
}

void ExportRendererSPPM::save_asset(AssetSaver &saver) const
{
    saver.write(int32_t(worker_count_->value()));
    saver.write(int32_t(forward_task_grid_size_->value()));
    saver.write(int32_t(forward_max_depth_->value()));
    saver.write(init_radius_->get_value());
    saver.write(int32_t(iteration_count_->value()));
    saver.write(int32_t(photons_per_iteration_->value()));
    saver.write(int32_t(photon_min_depth_->value()));
    saver.write(int32_t(photon_max_depth_->value()));
    saver.write(int32_t(photon_cont_prob_->value()));
    saver.write(alpha_->get_value());
    saver.write(int32_t(grid_res_->value()));
}

void ExportRendererSPPM::load_asset(AssetLoader &loader)
{
    worker_count_->setValue(int(loader.read<int32_t>()));
    forward_task_grid_size_->setValue(int(loader.read<int32_t>()));
    forward_max_depth_->setValue(int(loader.read<int32_t>()));
    init_radius_->set_value(loader.read<real>());
    iteration_count_->setValue(int(loader.read<int32_t>()));
    photons_per_iteration_->setValue(int(loader.read<int32_t>()));
    photon_min_depth_->setValue(int(loader.read<int32_t>()));
    photon_max_depth_->setValue(int(loader.read<int32_t>()));
    photon_cont_prob_->setValue(loader.read<real>());
    alpha_->set_value(loader.read<real>());
    grid_res_->setValue(int(loader.read<int32_t>()));
}

AGZ_EDITOR_END
