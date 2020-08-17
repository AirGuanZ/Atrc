#include <QGridLayout>
#include <QLabel>

#include <agz/editor/imexport/asset_loader.h>
#include <agz/editor/imexport/asset_saver.h>
#include <agz/editor/renderer/export/export_renderer_vol_bdpt.h>

AGZ_EDITOR_BEGIN

ExportRendererVolBDPT::ExportRendererVolBDPT(QWidget *parent)
    : ExportRendererWidget(parent)
{
    cam_max_vtx_cnt_ = new QSpinBox(this);
    lht_max_vtx_cnt_ = new QSpinBox(this);

    use_mis_ = new QCheckBox(this);
    spp_     = new QSpinBox(this);
    
    worker_count_   = new QSpinBox(this);
    task_grid_size_ = new QSpinBox(this);

    cam_max_vtx_cnt_->setRange(2, 200);
    cam_max_vtx_cnt_->setValue(6);

    lht_max_vtx_cnt_->setRange(2, 200);
    lht_max_vtx_cnt_->setValue(6);

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

    layout->addWidget(new QLabel("Camera Subpath Size"), row, 0);
    layout->addWidget(cam_max_vtx_cnt_, row, 1);

    layout->addWidget(new QLabel("Light Subpath Size"), ++row, 0);
    layout->addWidget(lht_max_vtx_cnt_, row, 1);

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

RC<tracer::ConfigGroup> ExportRendererVolBDPT::to_config() const
{
    auto grp = newRC<tracer::ConfigGroup>();

    grp->insert_str("type", "vol_bdpt");
    grp->insert_int("camera_max_depth", cam_max_vtx_cnt_->value());
    grp->insert_int("light_max_depth", lht_max_vtx_cnt_->value());
    grp->insert_bool("use_mis", use_mis_->isChecked());
    grp->insert_int("spp", spp_->value());
    grp->insert_int("worker_count", worker_count_->value());
    grp->insert_int("task_grid_size", task_grid_size_->value());

    return grp;
}

void ExportRendererVolBDPT::save_asset(AssetSaver &saver) const
{
    saver.write(int32_t(cam_max_vtx_cnt_->value()));
    saver.write(int32_t(lht_max_vtx_cnt_->value()));
    saver.write(int32_t(use_mis_->isChecked() ? 1 : 0));
    saver.write(int32_t(spp_->value()));
    saver.write(int32_t(worker_count_->value()));
    saver.write(int32_t(task_grid_size_->value()));
}

void ExportRendererVolBDPT::load_asset(AssetLoader &loader)
{
    cam_max_vtx_cnt_->setValue(int(loader.read<int32_t>()));
    lht_max_vtx_cnt_->setValue(int(loader.read<int32_t>()));
    use_mis_->setChecked(loader.read<int32_t>() != 0);
    spp_->setValue(int(loader.read<int32_t>()));
    worker_count_->setValue(int(loader.read<int32_t>()));
    task_grid_size_->setValue(int(loader.read<int32_t>()));
}

AGZ_EDITOR_END
