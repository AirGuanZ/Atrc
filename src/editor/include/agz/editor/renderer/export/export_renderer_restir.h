#pragma once

#include <QSpinBox>

#include <agz/editor/renderer/export/export_renderer.h>
#include <agz/tracer/utility/config.h>

AGZ_EDITOR_BEGIN

class ExportRendererReSTIR : public ExportRendererWidget
{
public:

    explicit ExportRendererReSTIR(QWidget *parent = nullptr);

    RC<tracer::ConfigGroup> to_config() const override;

    void save_asset(AssetSaver &saver) const override;

    void load_asset(AssetLoader &loader) override;

private:

    QSpinBox *spp_                = nullptr;
    QSpinBox *M_                  = nullptr;
    QSpinBox *I_                  = nullptr;
    QSpinBox *spatialReuseRadius_ = nullptr;
    QSpinBox *spatialReuseCount_  = nullptr;
    QSpinBox *worker_count_       = nullptr;
};

AGZ_EDITOR_END
