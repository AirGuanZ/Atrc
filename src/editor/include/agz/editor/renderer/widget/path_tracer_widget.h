#pragma once

#include <agz/editor/renderer/renderer_widget.h>

namespace Ui
{
    class PathTracer;
}

AGZ_EDITOR_BEGIN

class PathTracerWidget : public RendererWidget
{
public:

    explicit PathTracerWidget(QWidget *parent);

    ~PathTracerWidget();

    std::unique_ptr<Renderer> create_renderer(
        std::shared_ptr<tracer::Scene> scene, const Vec2i &framebuffer_size, bool enable_preview) const override;

private:

    int min_depth_  = 5;
    int max_depth_  = 10;
    real cont_prob_ = real(0.9);

    Ui::PathTracer *ui_;
};

class PathTracerWidgetCreator : public RendererWidgetCreator
{
public:

    std::string name() const override { return "Path Tracer"; }

    RendererWidget *create_widget(QWidget *parent) const override;
};

AGZ_EDITOR_END
