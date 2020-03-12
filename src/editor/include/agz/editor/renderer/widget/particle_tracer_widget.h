#pragma once

#include <QSlider>

#include <agz/editor/renderer/renderer_widget.h>

AGZ_EDITOR_BEGIN

class ParticleTracerWidget : public RendererWidget
{
public:

    explicit ParticleTracerWidget(QWidget *parent);

    Box<Renderer> create_renderer(
        RC<tracer::Scene> scene, const Vec2i &framebuffer_size,
        bool enable_preview) const override;

private:

    QSlider *min_depth_ = nullptr;
    QSlider *max_depth_ = nullptr;
    QSlider *cont_prob_ = nullptr;
    QSlider *particle_sample_count_ = nullptr;
};

class ParticleTracerWidgetCreator : public RendererWidgetCreator
{
public:

    std::string name() const override { return "Particle Tracer"; }

    RendererWidget *create_widget(QWidget *parent) const override;
};

AGZ_EDITOR_END
