#pragma once

#include <QSpinBox>

#include <agz/editor/renderer/export/export_renderer.h>
#include <agz/editor/ui/utility/color_holder.h>
#include <agz/editor/ui/utility/vec_input.h>

AGZ_EDITOR_BEGIN

class ExportRendererAO : public ExportRendererWidget
{
public:

    explicit ExportRendererAO(QWidget *parent = nullptr);

    RC<tracer::ConfigGroup> to_config() const override;

private:

    QSpinBox *ao_sps_ = nullptr;
    QSpinBox *spp_    = nullptr;

    RealInput *max_occlusion_distance_ = nullptr;

    ColorHolder *top_        = nullptr;
    ColorHolder *bottom_     = nullptr;
    ColorHolder *background_ = nullptr;

    QSpinBox *worker_count_   = nullptr;
    QSpinBox *task_grid_size_ = nullptr;
};

AGZ_EDITOR_END
