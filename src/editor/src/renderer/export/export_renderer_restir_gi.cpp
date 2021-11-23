#include <QGridLayout>
#include <QLabel>

#include <agz/editor/imexport/asset_loader.h>
#include <agz/editor/imexport/asset_saver.h>
#include <agz/editor/renderer/export/export_renderer_restir_gi.h>

AGZ_EDITOR_BEGIN

ExportRendererReSTIRGI::ExportRendererReSTIRGI(QWidget *parent)
    : ExportRendererWidget(parent)
{
    constexpr int I_MAX = (std::numeric_limits<int>::max)();

    int row = 0;
    QGridLayout *layout = new QGridLayout(this);

    auto add_widget = [&](const char *name, QWidget *widget)
    {
        layout->addWidget(new QLabel(name), row, 0);
        layout->addWidget(widget, row++, 1);
    };

    spp_ = new QSpinBox(this);
    spp_->setRange(1, I_MAX);
    spp_->setValue(16);
    add_widget("Samples per Pixel", spp_);

    min_depth_ = new QSpinBox(this);
    min_depth_->setRange(1, I_MAX);
    min_depth_->setValue(5);
    add_widget("Min Depth", min_depth_);

    max_depth_ = new QSpinBox(this);
    max_depth_->setRange(1, I_MAX);
    max_depth_->setValue(10);
    add_widget("Max Depth", max_depth_);

    specular_depth_ = new QSpinBox(this);
    specular_depth_->setRange(1, I_MAX);
    specular_depth_->setValue(20);
    add_widget("Specular Depth", specular_depth_);

    cont_prob_ = new RealSlider(this);
    cont_prob_->set_orientation(Qt::Horizontal);
    cont_prob_->set_range(0, 1);
    cont_prob_->set_value(real(0.8));
    add_widget("Cont Prob", cont_prob_);

    I_ = new QSpinBox(this);
    I_->setRange(0, I_MAX);
    I_->setValue(2);
    add_widget("I", I_);

    spatial_reuse_count_ = new QSpinBox(this);
    spatial_reuse_count_->setRange(0, I_MAX);
    spatial_reuse_count_->setValue(5);
    add_widget("Spatial Reuse Count", spatial_reuse_count_);

    spatial_reuse_radius_ = new QSpinBox(this);
    spatial_reuse_radius_->setRange(0, I_MAX);
    spatial_reuse_radius_->setValue(20);
    add_widget("Spatial Reuse Radius", spatial_reuse_radius_);

    worker_count_ = new QSpinBox(this);
    worker_count_->setValue(-1);
    add_widget("Worker Count", worker_count_);

    connect(min_depth_, qOverload<int>(&QSpinBox::valueChanged),
        [=](int new_min_depth)
    {
        if(new_min_depth > max_depth_->value())
        {
            max_depth_->blockSignals(true);
            max_depth_->setValue(new_min_depth);
            max_depth_->blockSignals(false);
        }
    });

    connect(max_depth_, qOverload<int>(&QSpinBox::valueChanged),
        [=](int new_max_depth)
    {
        if(new_max_depth < min_depth_->value())
        {
            min_depth_->blockSignals(true);
            min_depth_->setValue(new_max_depth);
            min_depth_->blockSignals(false);
        }
    });

    setContentsMargins(0, 0, 0, 0);
    layout->setContentsMargins(0, 0, 0, 0);
}

RC<tracer::ConfigGroup> ExportRendererReSTIRGI::to_config() const
{
    auto grp = newRC<tracer::ConfigGroup>();
    grp->insert_str("type", "restir-gi");
    grp->insert_int("spp", spp_->value());
    grp->insert_int("min_depth", min_depth_->value());
    grp->insert_int("max_depth", max_depth_->value());
    grp->insert_int("specular_depth", specular_depth_->value());
    grp->insert_real("cont_prob", cont_prob_->value());
    grp->insert_int("I", I_->value());
    grp->insert_int("spatial_reuse_count", spatial_reuse_count_->value());
    grp->insert_int("spatial_reuse_radius", spatial_reuse_radius_->value());
    grp->insert_int("worker_count", worker_count_->value());
    return grp;
}

void ExportRendererReSTIRGI::save_asset(AssetSaver &saver) const
{
    saver.write(int32_t(spp_->value()));
    saver.write(int32_t(min_depth_->value()));
    saver.write(int32_t(max_depth_->value()));
    saver.write(int32_t(specular_depth_->value()));
    saver.write(real(cont_prob_->value()));
    saver.write(int32_t(I_->value()));
    saver.write(int32_t(spatial_reuse_count_->value()));
    saver.write(int32_t(spatial_reuse_radius_->value()));
    saver.write(int32_t(worker_count_->value()));
}

void ExportRendererReSTIRGI::load_asset(AssetLoader &loader)
{
    spp_->setValue(loader.read<int32_t>());
    min_depth_->setValue(loader.read<int32_t>());
    max_depth_->setValue(loader.read<int32_t>());
    specular_depth_->setValue(loader.read<int32_t>());
    cont_prob_->set_value(loader.read<real>());
    I_->setValue(loader.read<int32_t>());
    spatial_reuse_count_->setValue(loader.read<int32_t>());
    spatial_reuse_radius_->setValue(loader.read<int32_t>());
    worker_count_->setValue(loader.read<int32_t>());
}

AGZ_EDITOR_END
