#include <QGridLayout>

#include <agz/editor/renderer/export/export_renderer_pt.h>

AGZ_EDITOR_BEGIN

ExportRendererPT::ExportRendererPT(QWidget *parent)
{
    min_depth_ = new QSlider(this);
    max_depth_ = new QSlider(this);

    cont_prob_ = new RealSlider(this);

    use_mis_ = new QCheckBox(this);
    spp_     = new QSpinBox(this);

    worker_count_   = new QSpinBox(this);
    task_grid_size_ = new QSpinBox(this);

    min_depth_->setOrientation(Qt::Horizontal);
    min_depth_->setRange(1, 20);
    min_depth_->setValue(5);

    max_depth_->setOrientation(Qt::Horizontal);
    max_depth_->setRange(1, 20);
    max_depth_->setValue(10);

    connect(min_depth_, &QSlider::valueChanged, [=](int new_min_depth)
    {
        if(new_min_depth > max_depth_->value())
        {
            max_depth_->blockSignals(true);
            max_depth_->setValue(new_min_depth);
            max_depth_->blockSignals(false);
        }
    });

    connect(max_depth_, &QSlider::valueChanged, [=](int new_max_depth)
    {
        if(new_max_depth < min_depth_->value())
        {
            min_depth_->blockSignals(true);
            min_depth_->setValue(new_max_depth);
            min_depth_->blockSignals(false);
        }
    });

    cont_prob_->set_orientation(Qt::Horizontal);
    cont_prob_->set_range(0, 1);
    cont_prob_->set_value(real(0.5));

    use_mis_->setChecked(true);
    spp_->setRange(1, (std::numeric_limits<int>::max)());
    spp_->setValue(100);
    
    worker_count_->setRange((std::numeric_limits<int>::lowest)(),
                            (std::numeric_limits<int>::max)());
    worker_count_->setValue(-1);

    task_grid_size_->setRange(1, 512);
    task_grid_size_->setValue(32);

    QGridLayout *layout = new QGridLayout(this);
    int row = 0;

    layout->addWidget(new QLabel("Min Depth"), row, 0);
    layout->addWidget(min_depth_, row, 1);

    layout->addWidget(new QLabel("Max Depth"), ++row, 0);
    layout->addWidget(max_depth_, row, 1);

    layout->addWidget(new QLabel("Cont Prob"), ++row, 0);
    layout->addWidget(cont_prob_, row, 1);

    layout->addWidget(new QLabel("Use MIS"), ++row, 0);
    layout->addWidget(use_mis_, row, 1);

    layout->addWidget(new QLabel("Samples per Pixel"), ++row, 0);
    layout->addWidget(spp_, row, 1);

    layout->addWidget(new QLabel("Thread Count"), ++row, 0);
    layout->addWidget(worker_count_, row, 1);

    layout->addWidget(new QLabel("Thread Task Size"), ++row, 0);
    layout->addWidget(task_grid_size_, row, 1);

    setContentsMargins(0, 0, 0, 0);
    layout->setContentsMargins(0, 0, 0, 0);
}

std::shared_ptr<tracer::ConfigGroup> ExportRendererPT::to_config() const
{
    auto grp = std::make_shared<tracer::ConfigGroup>();

    grp->insert_str("type", "pt");
    grp->insert_int("min_depth", min_depth_->value());
    grp->insert_int("max_depth", max_depth_->value());
    grp->insert_real("cont_prob", cont_prob_->value());
    grp->insert_bool("use_mis", use_mis_->isChecked());
    grp->insert_int("spp", spp_->value());
    grp->insert_int("worker_count", worker_count_->value());
    grp->insert_int("task_grid_size", task_grid_size_->value());

    return grp;
}

AGZ_EDITOR_END
