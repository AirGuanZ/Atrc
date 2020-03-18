#pragma once

#include <QCheckBox>

#include <agz/editor/renderer/renderer_widget.h>

AGZ_EDITOR_BEGIN

class BDPTRendererWidget : public RendererWidget
{
public:

    explicit BDPTRendererWidget(QWidget *parent);

    Box<Renderer> create_renderer(
        RC<tracer::Scene> scene, const Vec2i &framebuffer_size,
        bool enable_preview) const override;

    void save_asset(AssetSaver &saver) const override;

    void load_asset(AssetLoader &loader) override;

private:

    QSlider *max_cam_depth_ = nullptr;
    QSlider *max_lht_depth_ = nullptr;
};

class BDPTRendererWidgetCreator : public RendererWidgetCreator
{
public:

    std::string name() const override { return "BDPT"; }

    RendererWidget *create_widget(QWidget *parent) const override;
};

AGZ_EDITOR_END
