#include <QVBoxLayout>

#include <agz/editor/renderer/path_tracer.h>
#include <agz/editor/renderer/renderer_widget/path_tracer_widget.h>

#include "ui_PathTracer.h"

AGZ_EDITOR_BEGIN

PathTracerWidget::PathTracerWidget(QWidget *parent)
    : RendererWidget(parent), ui_(new Ui::PathTracer)
{
    ui_->setupUi(this);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(ui_->formLayoutWidget);

    ui_->min_depth_slider->setValue(min_depth_);
    ui_->max_depth_slider->setValue(max_depth_);
    ui_->cont_prob_slider->setValue(static_cast<int>(cont_prob_ * 10));
    
    connect(ui_->min_depth_slider, &QSlider::valueChanged, [=](int value)
    {
        min_depth_ = value;
        if(min_depth_ > max_depth_)
            ui_->max_depth_slider->setValue(min_depth_);
        emit change_renderer_params();
    });

    connect(ui_->max_depth_slider, &QSlider::valueChanged, [=](int value)
    {
        max_depth_ = value;
        if(min_depth_ > max_depth_)
            ui_->min_depth_slider->setValue(max_depth_);
        emit change_renderer_params();
    });

    connect(ui_->cont_prob_slider, &QSlider::valueChanged, [=](int value)
    {
        cont_prob_ = value / real(10);
        emit change_renderer_params();
    });
}

std::unique_ptr<Renderer> PathTracerWidget::create_renderer(std::shared_ptr<tracer::Scene> scene, const Vec2i &framebuffer_size) const
{
    PathTracingParams params = { -1, min_depth_, max_depth_, cont_prob_ };
    return std::make_unique<PathTracer>(params, framebuffer_size.x, framebuffer_size.y, std::move(scene));
}

RendererWidget *PathTracerWidgetCreator::create_widget(QWidget *parent) const
{
    return new PathTracerWidget(parent);
}

AGZ_EDITOR_END
