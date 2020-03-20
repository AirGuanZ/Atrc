#pragma once

#include <QSpinBox>

#include <agz/editor/renderer/export/export_renderer.h>
#include <agz/editor/ui/utility/real_slider.h>
#include <agz/editor/ui/utility/vec_input.h>

AGZ_EDITOR_BEGIN

class ExportRendererSPPM : public ExportRendererWidget
{
public:

    explicit ExportRendererSPPM(QWidget *parent = nullptr);

    RC<tracer::ConfigGroup> to_config() const override;

    void save_asset(AssetSaver &saver) const override;

    void load_asset(AssetLoader &loader) override;

private:

    QSpinBox *worker_count_ = nullptr;

    QSpinBox *forward_task_grid_size_ = nullptr;

    QSlider *forward_max_depth_ = nullptr;

    RealInput *init_radius_ = nullptr;

    QSpinBox *iteration_count_       = nullptr;
    QSpinBox *photons_per_iteration_ = nullptr;

    QSlider *photon_min_depth_ = nullptr;
    QSlider *photon_max_depth_ = nullptr;
    QSlider *photon_cont_prob_ = nullptr;

    RealInput *alpha_ = nullptr;

    QSpinBox *grid_res_ = nullptr;
};

AGZ_EDITOR_END
