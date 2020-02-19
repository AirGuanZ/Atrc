#include <agz/editor/texture2d/constant2d.h>

AGZ_EDITOR_BEGIN

Constant2DWidget::Constant2DWidget(const Spectrum &init_color)
{
    layout_       = new QVBoxLayout(this);
    color_holder_ = new ColorHolder(init_color, this);
    layout_->addWidget(color_holder_);

    setContentsMargins(0, 0, 0, 0);
    layout_->setContentsMargins(0, 0, 0, 0);

    connect(color_holder_, &ColorHolder::change_color, [=](const Spectrum &)
    {
        set_dirty_flag();
    });

    tracer_object_ = tracer::create_constant2d_texture({}, color_holder_->get_color());
}

Texture2DWidget *Constant2DWidget::clone()
{
    return new Constant2DWidget(color_holder_->get_color());
}

QPixmap Constant2DWidget::get_thumbnail(int width, int height) const
{
    QImage image(1, 1, QImage::Format::Format_RGB888);
    image.setPixelColor(0, 0, color_holder_->get_qcolor());

    QPixmap pixmap;
    pixmap.convertFromImage(image);

    return pixmap.scaled(width, height);
}

void Constant2DWidget::update_tracer_object_impl()
{
    tracer_object_ = tracer::create_constant2d_texture({}, color_holder_->get_color());
}

ResourceWidget<tracer::Texture2D> *Constant2DCreator::create_widget(ObjectContext &obj_ctx) const
{
    return new Constant2DWidget;
}

AGZ_EDITOR_END
