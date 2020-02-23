#pragma once

#include <QCheckBox>

#include <agz/editor/renderer/renderer_widget.h>

AGZ_EDITOR_BEGIN

class BDPTRendererWidget : public RendererWidget
{
public:

    explicit BDPTRendererWidget(QWidget *parent);

    std::unique_ptr<Renderer> create_renderer(
        std::shared_ptr<tracer::Scene> scene, const Vec2i &framebuffer_size, bool enable_preview) const override;

private:

    QSlider   *max_cam_depth_ = nullptr;
    QSlider   *max_lht_depth_ = nullptr;
    QCheckBox *use_mis_ = nullptr;
};

class BDPTRendererWidgetCreator : public RendererWidgetCreator
{
public:

    std::string name() const override { return "BDPT"; }

    RendererWidget *create_widget(QWidget *parent) const override;
};

AGZ_EDITOR_END
