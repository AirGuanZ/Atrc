#include <QGridLayout>
#include <agz/editor/renderer/export/export_renderer_ao.h>

AGZ_EDITOR_BEGIN

ExportRendererAO::ExportRendererAO(QWidget *parent)
    : ExportRendererWidget(parent)
{
    ao_sps_ = new QSpinBox(this);
    spp_    = new QSpinBox(this);

    max_occlusion_distance_ = new RealInput(this);

    top_        = new ColorHolder({ 1, 1, 1 }, this);
    bottom_     = new ColorHolder({ 0, 0, 0 }, this);
    background_ = new ColorHolder({ 0, 0, 0 }, this);

    worker_count_   = new QSpinBox(this);
    task_grid_size_ = new QSpinBox(this);

    ao_sps_->setRange(1, 100000);
    ao_sps_->setValue(4);

    spp_->setRange(1, (std::numeric_limits<int>::max)());
    spp_->setValue(100);

    max_occlusion_distance_->set_value(1);

    worker_count_->setRange((std::numeric_limits<int>::lowest)(),
                            (std::numeric_limits<int>::max)());
    worker_count_->setValue(-1);

    task_grid_size_->setRange(1, 512);
    task_grid_size_->setValue(32);

    QGridLayout *layout = new QGridLayout(this);
    int row = 0;

    layout->addWidget(new QLabel("AO Samples per Sample"), row, 0);
    layout->addWidget(ao_sps_, row, 1);

    layout->addWidget(new QLabel("Samples per Pixel"), ++row, 0);
    layout->addWidget(spp_, row, 1);

    layout->addWidget(new QLabel("High Color"), ++row, 0);
    layout->addWidget(top_, row, 1);

    layout->addWidget(new QLabel("Low Color"), ++row, 0);
    layout->addWidget(bottom_, row, 1);

    layout->addWidget(new QLabel("Background Color"), ++row, 0);
    layout->addWidget(background_, row, 1);

    layout->addWidget(new QLabel("Max Occlusion Distance"), ++row, 0);
    layout->addWidget(max_occlusion_distance_, row, 1);

    layout->addWidget(new QLabel("Thread Count"), ++row, 0);
    layout->addWidget(worker_count_, row, 1);

    layout->addWidget(new QLabel("Thread Task Size"), ++row, 0);
    layout->addWidget(task_grid_size_, row, 1);

    setContentsMargins(0, 0, 0, 0);
    layout->setContentsMargins(0, 0, 0, 0);
}

std::shared_ptr<tracer::ConfigGroup> ExportRendererAO::to_config() const
{
    auto grp = std::make_shared<tracer::ConfigGroup>();

    grp->insert_str("type", "ao");
    grp->insert_int("worker_count", worker_count_->value());
    grp->insert_int("task_grid_size", task_grid_size_->value());
    grp->insert_int("ao_sample_count", ao_sps_->value());
    grp->insert_child("low_color", tracer::ConfigArray::from_spectrum(bottom_->get_color()));
    grp->insert_child("high_color", tracer::ConfigArray::from_spectrum(top_->get_color()));
    grp->insert_child("background_color", tracer::ConfigArray::from_spectrum(background_->get_color()));
    grp->insert_real("max_occlusion_distance", max_occlusion_distance_->get_value());
    grp->insert_int("spp", spp_->value());

    return grp;
}

AGZ_EDITOR_END
