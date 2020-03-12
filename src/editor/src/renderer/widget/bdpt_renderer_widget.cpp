#include <QGridLayout>
#include <QLabel>

#include <agz/editor/renderer/widget/bdpt_renderer_widget.h>
#include <agz/editor/renderer/bdpt.h>

AGZ_EDITOR_BEGIN

BDPTRendererWidget::BDPTRendererWidget(QWidget *parent)
    : RendererWidget(parent)
{
    max_cam_depth_ = new QSlider(this);
    max_lht_depth_ = new QSlider(this);

    QLabel *max_cam_depth_text = new QLabel("Max Camera Depth", this);
    QLabel *max_lht_depth_text = new QLabel("Max Light  Depth", this);

    QLabel *max_cam_depth_display = new QLabel(this);
    QLabel *max_lht_depth_display = new QLabel(this);

    max_cam_depth_display->setSizePolicy(
        QSizePolicy::Fixed, QSizePolicy::Preferred);
    max_lht_depth_display->setSizePolicy(
        QSizePolicy::Fixed, QSizePolicy::Preferred);

    max_cam_depth_display->setAlignment(Qt::AlignCenter);
    max_lht_depth_display->setAlignment(Qt::AlignCenter);

    max_cam_depth_->setOrientation(Qt::Horizontal);
    max_lht_depth_->setOrientation(Qt::Horizontal);

    max_cam_depth_->setRange(1, 20);
    max_lht_depth_->setRange(1, 20);

    max_cam_depth_->setValue(5);
    max_lht_depth_->setValue(5);

    max_cam_depth_display->setText("5");
    max_lht_depth_display->setText("5");

    QGridLayout *layout = new QGridLayout(this);

    layout->addWidget(max_cam_depth_text, 0, 0);
    layout->addWidget(max_cam_depth_display, 0, 1);
    layout->addWidget(max_cam_depth_, 0, 2);

    layout->addWidget(max_lht_depth_text, 1, 0);
    layout->addWidget(max_lht_depth_display, 1, 1);
    layout->addWidget(max_lht_depth_, 1, 2);

    connect(max_cam_depth_, &QSlider::valueChanged, [=](int new_value)
    {
        max_cam_depth_display->setText(QString::number(new_value));
        emit change_renderer_params();
    });

    connect(max_lht_depth_, &QSlider::valueChanged, [=](int new_value)
    {
        max_lht_depth_display->setText(QString::number(new_value));
        emit change_renderer_params();
    });
}

Box<Renderer> BDPTRendererWidget::create_renderer(
    RC<tracer::Scene> scene, const Vec2i &framebuffer_size,
    bool enable_preview) const
{
    BDPTRenderer::Params params = {
        -2, 32,
        max_cam_depth_->value(),
        max_lht_depth_->value(),
        enable_preview
    };
    return newBox<BDPTRenderer>(
        params, framebuffer_size.x, framebuffer_size.y, std::move(scene));
}

RendererWidget *BDPTRendererWidgetCreator::create_widget(QWidget *parent) const
{
    return new BDPTRendererWidget(parent);
}

AGZ_EDITOR_END
