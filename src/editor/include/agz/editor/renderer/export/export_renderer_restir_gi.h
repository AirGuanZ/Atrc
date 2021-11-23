#pragma once

#include <QSpinBox>

#include <agz/editor/renderer/export/export_renderer.h>
#include <agz/editor/ui/utility/real_slider.h>
#include <agz/tracer/utility/config.h>

AGZ_EDITOR_BEGIN

class ExportRendererReSTIRGI : public ExportRendererWidget
{
public:

    explicit ExportRendererReSTIRGI(QWidget *parent = nullptr);

    RC<tracer::ConfigGroup> to_config() const override;

    void save_asset(AssetSaver &saver) const override;

    void load_asset(AssetLoader &loader) override;

private:

    QSpinBox *spp_ = nullptr;

    QSpinBox   *min_depth_      = nullptr;
    QSpinBox   *max_depth_      = nullptr;
    QSpinBox   *specular_depth_ = nullptr;
    RealSlider *cont_prob_      = nullptr;

    QSpinBox *I_                    = nullptr;
    QSpinBox *spatial_reuse_count_  = nullptr;
    QSpinBox *spatial_reuse_radius_ = nullptr;

    QSpinBox *worker_count_ = nullptr;
};

AGZ_EDITOR_END
