#pragma once

#include <QColor>
#include <QPushButton>

#include <agz/editor/renderer/renderer_widget.h>

namespace Ui
{
    class AO;
}

AGZ_EDITOR_BEGIN

class AOWidget : public RendererWidget
{
public:

    explicit AOWidget(QWidget *parent);

    Box<Renderer> create_renderer(
        RC<tracer::Scene> scene, const Vec2i &framebuffer_size,
        bool enable_preview) const override;

private:

    static void set_button_color(QPushButton *button, const QColor &color);

    int ao_sample_count_ = 4;
    real occlusion_distance_ = 1;

    QColor background_color_ = QColor(Qt::black);
    QColor low_color_        = QColor(Qt::black);
    QColor high_color_       = QColor(Qt::white);

    Ui::AO *ui_;
};

class AOWidgetCreator : public RendererWidgetCreator
{
public:

    std::string name() const override { return "Ambient Occlusion"; }

    RendererWidget *create_widget(QWidget *parent) const override;
};

AGZ_EDITOR_END
