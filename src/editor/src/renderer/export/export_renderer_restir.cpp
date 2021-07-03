#include <QGridLayout>
#include <QLabel>

#include <agz/editor/imexport/asset_loader.h>
#include <agz/editor/imexport/asset_saver.h>
#include <agz/editor/renderer/export/export_renderer_restir.h>

AGZ_EDITOR_BEGIN

ExportRendererReSTIR::ExportRendererReSTIR(QWidget *parent)
    : ExportRendererWidget(parent)
{
    spp_                = new QSpinBox(this);
    M_                  = new QSpinBox(this);
    I_                  = new QSpinBox(this);
    spatialReuseRadius_ = new QSpinBox(this);
    spatialReuseCount_  = new QSpinBox(this);
    worker_count_       = new QSpinBox(this);

    spp_->setRange(1, (std::numeric_limits<int>::max)());
    M_->setRange(1, (std::numeric_limits<int>::max)());
    I_->setRange(0, (std::numeric_limits<int>::max)());
    spatialReuseRadius_->setRange(1, (std::numeric_limits<int>::max)());
    spatialReuseCount_->setRange(0, (std::numeric_limits<int>::max)());

    spp_->setValue(16);
    M_->setValue(32);
    I_->setValue(2);
    spatialReuseRadius_->setValue(20);
    spatialReuseCount_->setValue(5);
    worker_count_->setValue(-1);

    QGridLayout *layout = new QGridLayout(this);
    int row = 0;

    layout->addWidget(new QLabel("Samples per Pixel"), row, 0);
    layout->addWidget(spp_, row, 1);

    layout->addWidget(new QLabel("M"), ++row, 0);
    layout->addWidget(M_, row, 1);

    layout->addWidget(new QLabel("I"), ++row, 0);
    layout->addWidget(I_, row, 1);

    layout->addWidget(new QLabel("Spatial Reuse Radius"), ++row, 0);
    layout->addWidget(spatialReuseRadius_, row, 1);

    layout->addWidget(new QLabel("Spatial Reuse Count"), ++row, 0);
    layout->addWidget(spatialReuseCount_, row, 1);

    layout->addWidget(new QLabel("Worker Count"), ++row, 0);
    layout->addWidget(worker_count_, row, 1);

    setContentsMargins(0, 0, 0, 0);
    layout->setContentsMargins(0, 0, 0, 0);
}

RC<tracer::ConfigGroup> ExportRendererReSTIR::to_config() const
{
    auto grp = newRC<tracer::ConfigGroup>();
    grp->insert_str("type", "restir");
    grp->insert_int("spp", spp_->value());
    grp->insert_int("M", M_->value());
    grp->insert_int("I", I_->value());
    grp->insert_int("spatial_reuse_radius", spatialReuseRadius_->value());
    grp->insert_int("spatial_reuse_count", spatialReuseCount_->value());
    grp->insert_int("worker_count", worker_count_->value());
    return grp;
}

void ExportRendererReSTIR::save_asset(AssetSaver &saver) const
{
    saver.write(int32_t(spp_->value()));
    saver.write(int32_t(M_->value()));
    saver.write(int32_t(I_->value()));
    saver.write(int32_t(spatialReuseRadius_->value()));
    saver.write(int32_t(spatialReuseCount_->value()));
    saver.write(int32_t(worker_count_->value()));
}

void ExportRendererReSTIR::load_asset(AssetLoader &loader)
{
    spp_->setValue(int(loader.read<int32_t>()));
    M_->setValue(int(loader.read<int32_t>()));
    I_->setValue(int(loader.read<int32_t>()));
    spatialReuseRadius_->setValue(int(loader.read<int32_t>()));
    spatialReuseCount_->setValue(int(loader.read<int32_t>()));
    worker_count_->setValue(int(loader.read<int32_t>()));
}

AGZ_EDITOR_END
