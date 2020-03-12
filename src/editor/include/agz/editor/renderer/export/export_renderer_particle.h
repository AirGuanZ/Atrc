#pragma once

#include <QSpinBox>

#include <agz/editor/renderer/export/export_renderer.h>
#include <agz/editor/ui/utility/real_slider.h>

AGZ_EDITOR_BEGIN

class ExportRendererParticle : public ExportRendererWidget
{
public:

    explicit ExportRendererParticle(QWidget *parent = nullptr);

    RC<tracer::ConfigGroup> to_config() const override;

private:

    QSpinBox *min_depth_ = nullptr;
    QSpinBox *max_depth_ = nullptr;
    RealSlider *cont_prob_ = nullptr;

    QSpinBox *particles_per_task_  = nullptr;
    QSpinBox *particle_task_count_ = nullptr;

    QSpinBox *forward_spp_       = nullptr;
    QSpinBox *forward_task_size_ = nullptr;

    QSpinBox *worker_count_ = nullptr;
};

AGZ_EDITOR_END
