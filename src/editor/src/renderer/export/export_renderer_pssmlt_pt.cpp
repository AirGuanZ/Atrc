#include <QGridLayout>
#include <QLabel>

#include <agz/editor/imexport/asset_loader.h>
#include <agz/editor/imexport/asset_saver.h>
#include <agz/editor/renderer/export/export_renderer_pssmlt_pt.h>

AGZ_EDITOR_BEGIN

ExportRendererPSSMLTPT::ExportRendererPSSMLTPT(QWidget *parent)
    : ExportRendererWidget(parent)
{
    QGridLayout *layout = new QGridLayout(this);
    int row = -1;

    // worker count

    worker_count_ = new QSpinBox(this);
    worker_count_->setRange(
        (std::numeric_limits<int>::lowest)(),
        (std::numeric_limits<int>::max)());
    worker_count_->setValue(-1);

    layout->addWidget(new QLabel("Thread Count"), ++row, 0, 1, 1);
    layout->addWidget(worker_count_, row, 1, 1, 2);

    // min/max depth

    auto min_depth_display = new QLabel(this);
    auto max_depth_display = new QLabel(this);

    min_depth_ = new QSlider(this);
    max_depth_ = new QSlider(this);
    min_depth_->setOrientation(Qt::Horizontal);
    max_depth_->setOrientation(Qt::Horizontal);
    min_depth_->setRange(1, 20);
    max_depth_->setRange(1, 20);

    connect(min_depth_, &QSlider::valueChanged, [=](int new_value)
    {
        min_depth_display->setText(QString::number(new_value));
    });

    connect(max_depth_, &QSlider::valueChanged, [=](int new_value)
    {
        max_depth_display->setText(QString::number(new_value));
    });

    min_depth_->setValue(5);
    max_depth_->setValue(10);

    connect(min_depth_, &QSlider::valueChanged, [=](int new_val)
    {
        if(new_val > max_depth_->value())
        {
            max_depth_->blockSignals(true);
            max_depth_->setValue(new_val);
            max_depth_->blockSignals(false);

            max_depth_display->setText(QString::number(new_val));
        }
    });

    connect(max_depth_, &QSlider::valueChanged, [=](int new_val)
    {
        if(new_val < min_depth_->value())
        {
            min_depth_->blockSignals(true);
            min_depth_->setValue(new_val);
            min_depth_->blockSignals(false);

            min_depth_display->setText(QString::number(new_val));
        }
    });

    layout->addWidget(new QLabel("Min Depth"), ++row, 0, 1, 1);
    layout->addWidget(min_depth_display, row, 1, 1, 1);
    layout->addWidget(min_depth_, row, 2, 1, 1);

    layout->addWidget(new QLabel("Max Depth"), ++row, 0, 1, 1);
    layout->addWidget(max_depth_display, row, 1, 1, 1);
    layout->addWidget(max_depth_, row, 2, 1, 1);

    // cont prob

    auto cont_prob_display = new QLabel(this);
    cont_prob_display->setText("0.9");

    cont_prob_ = new QSlider(this);
    cont_prob_->setOrientation(Qt::Horizontal);
    cont_prob_->setRange(1, 10);

    connect(cont_prob_, &QSlider::valueChanged, [=](int new_val)
    {
        cont_prob_display->setText(QString::number(new_val / real(10)));
    });

    cont_prob_->setValue(9);

    layout->addWidget(new QLabel("Cont Prob"), ++row, 0, 1, 1);
    layout->addWidget(cont_prob_display, row, 1, 1, 1);
    layout->addWidget(cont_prob_, row, 2, 1, 1);

    // use mis

    use_mis_ = new QCheckBox(this);
    use_mis_->setChecked(true);

    layout->addWidget(new QLabel("Use MIS"), ++row, 0, 1, 1);
    layout->addWidget(use_mis_, row, 1, 1, 2);

    // startup sample count

    startup_sample_count_ = new QSpinBox(this);
    startup_sample_count_->setRange(1, (std::numeric_limits<int>::max)());
    startup_sample_count_->setValue(100000);

    layout->addWidget(new QLabel("Startup Sample Count"), ++row, 0, 1, 1);
    layout->addWidget(startup_sample_count_, row, 1, 1, 2);

    // mut per pixel

    mut_per_pixel_ = new QSpinBox(this);
    mut_per_pixel_->setRange(1, (std::numeric_limits<int>::max)());
    mut_per_pixel_->setValue(100);

    layout->addWidget(new QLabel("Mutations per Pixel"), ++row, 0, 1, 1);
    layout->addWidget(mut_per_pixel_, row, 1, 1, 2);

    // chain count

    chain_count_ = new QSpinBox(this);
    chain_count_->setRange(1, (std::numeric_limits<int>::max)());
    chain_count_->setValue(1000);

    layout->addWidget(new QLabel("Markov Chain Count"), ++row, 0, 1, 1);
    layout->addWidget(chain_count_, row, 1, 1, 2);

    // sigma

    sigma_ = new RealInput(this);
    sigma_->set_value(real(0.01));

    layout->addWidget(new QLabel("Small Mutation Size"), ++row, 0, 1, 1);
    layout->addWidget(sigma_, row, 1, 1, 2);

    // large step prob

    large_step_prob_ = new RealSlider(this);
    large_step_prob_->set_range(real(0.01), real(0.99));
    large_step_prob_->set_orientation(Qt::Horizontal);

    auto large_step_prob_text = new QLabel(this);
    connect(large_step_prob_, &RealSlider::change_value, [=](real new_val)
    {
        large_step_prob_text->setText(QString::number(new_val));
    });

    large_step_prob_->set_value(real(0.35));

    layout->addWidget(new QLabel("Large Step Prob"), ++row, 0, 1, 1);
    layout->addWidget(large_step_prob_text, row, 1, 1, 1);
    layout->addWidget(large_step_prob_, row, 2, 1, 1);

    setContentsMargins(0, 0, 0, 0);
    layout->setContentsMargins(0, 0, 0, 0);
}

RC<tracer::ConfigGroup> ExportRendererPSSMLTPT::to_config() const
{
    auto grp = newRC<tracer::ConfigGroup>();

    grp->insert_str("type", "pssmlt_pt");
    grp->insert_int("worker_count", worker_count_->value());
    grp->insert_int("min_depth", min_depth_->value());
    grp->insert_int("max_depth", max_depth_->value());
    grp->insert_real("cont_prob", cont_prob_->value() / real(10));
    grp->insert_bool("use_mis", use_mis_->isChecked());
    grp->insert_int("startup_sample_count", startup_sample_count_->value());
    grp->insert_int("mut_per_pixel", mut_per_pixel_->value());
    grp->insert_real("sigma", sigma_->get_value());
    grp->insert_real("large_step_prob", large_step_prob_->value());
    grp->insert_int("chain_count", chain_count_->value());

    return grp;
}

void ExportRendererPSSMLTPT::save_asset(AssetSaver &saver) const
{
    saver.write(int32_t(worker_count_->value()));
    saver.write(int32_t(min_depth_->value()));
    saver.write(int32_t(max_depth_->value()));
    saver.write(cont_prob_->value());
    saver.write(int32_t(use_mis_->isChecked() ? 1 : 0));
    saver.write(int32_t(startup_sample_count_->value()));
    saver.write(int32_t(mut_per_pixel_->value()));
    saver.write(sigma_->get_value());
    saver.write(large_step_prob_->value());
    saver.write(int32_t(chain_count_->value()));
}

void ExportRendererPSSMLTPT::load_asset(AssetLoader &loader)
{
    worker_count_->setValue(int(loader.read<int32_t>()));
    min_depth_->setValue(int(loader.read<int32_t>()));
    max_depth_->setValue(int(loader.read<int32_t>()));
    cont_prob_->setValue(loader.read<real>());
    use_mis_->setChecked(loader.read<int32_t>() != 0);
    startup_sample_count_->setValue(int(loader.read<int32_t>()));
    mut_per_pixel_->setValue(int(loader.read<int32_t>()));
    sigma_->set_value(loader.read<real>());
    large_step_prob_->set_value(loader.read<real>());
    chain_count_->setValue(int(loader.read<int32_t>()));
}

AGZ_EDITOR_END
