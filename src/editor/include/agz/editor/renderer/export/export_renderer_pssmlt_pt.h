#pragma once

#include <QCheckBox>
#include <QSpinBox>

#include <agz/editor/renderer/export/export_renderer.h>
#include <agz/editor/ui/utility/real_slider.h>
#include <agz/editor/ui/utility/vec_input.h>

AGZ_EDITOR_BEGIN

class ExportRendererPSSMLTPT : public ExportRendererWidget
{
public:

    explicit ExportRendererPSSMLTPT(QWidget *parent = nullptr);

    RC<tracer::ConfigGroup> to_config() const override;

private:

    QSpinBox *worker_count_ = nullptr;

    QSlider *min_depth_ = nullptr;
    QSlider *max_depth_ = nullptr;
    QSlider *cont_prob_ = nullptr;

    QCheckBox *use_mis_ = nullptr;

    QSpinBox *startup_sample_count_ = nullptr;
    QSpinBox *mut_per_pixel_ = nullptr;

    RealInput *sigma_ = nullptr;
    RealSlider *large_step_prob_ = nullptr;

    QSpinBox *chain_count_ = nullptr;
};

AGZ_EDITOR_END
