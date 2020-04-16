#pragma once

#include <QCheckBox>
#include <QSpinBox>

#include <agz/editor/renderer/export/export_renderer.h>

AGZ_EDITOR_BEGIN

class ExportRendererVolBDPT : public ExportRendererWidget
{
public:

    explicit ExportRendererVolBDPT(QWidget *parent = nullptr);

    RC<tracer::ConfigGroup> to_config() const override;

    void save_asset(AssetSaver &saver) const override;

    void load_asset(AssetLoader &loader) override;

private:

    QSpinBox *cam_max_vtx_cnt_ = nullptr;
    QSpinBox *lht_max_vtx_cnt_ = nullptr;

    QCheckBox *use_mis_ = nullptr;
    QSpinBox  *spp_     = nullptr;

    QSpinBox *worker_count_   = nullptr;
    QSpinBox *task_grid_size_ = nullptr;
};

AGZ_EDITOR_END
