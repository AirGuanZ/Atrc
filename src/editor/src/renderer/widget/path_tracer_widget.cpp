#include <QVBoxLayout>

#include <agz/editor/imexport/asset_loader.h>
#include <agz/editor/imexport/asset_saver.h>
#include <agz/editor/renderer/path_tracer.h>
#include <agz/editor/renderer/widget/path_tracer_widget.h>

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

    ui_->display_min_depth->setText(QString::number(min_depth_));
    ui_->display_max_depth->setText(QString::number(max_depth_));
    ui_->display_cont_prob->setText(QString::number(cont_prob_));
    
    connect(ui_->min_depth_slider, &QSlider::valueChanged, [=](int value)
    {
        min_depth_ = value;
        ui_->display_min_depth->setText(QString::number(value));
        if(min_depth_ > max_depth_)
            ui_->max_depth_slider->setValue(min_depth_);
        emit change_renderer_params();
    });

    connect(ui_->max_depth_slider, &QSlider::valueChanged, [=](int value)
    {
        max_depth_ = value;
        ui_->display_max_depth->setText(QString::number(value));
        if(min_depth_ > max_depth_)
            ui_->min_depth_slider->setValue(max_depth_);
        emit change_renderer_params();
    });

    connect(ui_->cont_prob_slider, &QSlider::valueChanged, [=](int value)
    {
        cont_prob_ = value / real(10);
        ui_->display_cont_prob->setText(QString::number(cont_prob_));
        emit change_renderer_params();
    });
}

PathTracerWidget::~PathTracerWidget()
{
    delete ui_;
}

Box<Renderer> PathTracerWidget::create_renderer(
    RC<tracer::Scene> scene, const Vec2i &framebuffer_size,
    bool enable_preview) const
{
    PathTracer::Params params = {
        -2, 32,
        min_depth_, max_depth_, cont_prob_,
        ui_->fast_preview->isChecked(),
        enable_preview
    };
    return newBox<PathTracer>(
        params, framebuffer_size.x, framebuffer_size.y, std::move(scene));
}

void PathTracerWidget::save_asset(AssetSaver &saver) const
{
    saver.write(int32_t(min_depth_));
    saver.write(int32_t(max_depth_));
    saver.write(cont_prob_);
    saver.write(int32_t(ui_->fast_preview->isChecked() ? 1 : 0));
}

void PathTracerWidget::load_asset(AssetLoader &loader)
{
    ui_->min_depth_slider->setValue(int(loader.read<int32_t>()));
    ui_->max_depth_slider->setValue(int(loader.read<int32_t>()));
    ui_->cont_prob_slider->setValue(int(loader.read<real>() * 10));
    ui_->fast_preview->setChecked(loader.read<int32_t>() != 0);

    emit change_renderer_params();
}

RendererWidget *PathTracerWidgetCreator::create_widget(QWidget *parent) const
{
    return new PathTracerWidget(parent);
}

AGZ_EDITOR_END
