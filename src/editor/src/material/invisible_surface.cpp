#include <agz/editor/material/invisible_surface.h>

AGZ_EDITOR_BEGIN

InvisibleSurfaceWidget::InvisibleSurfaceWidget()
{
    do_update_tracer_object();
}

ResourceWidget<tracer::Material> *InvisibleSurfaceWidget::clone()
{
    return new InvisibleSurfaceWidget;
}

QPixmap InvisibleSurfaceWidget::get_thumbnail(int width, int height) const
{
    return QPixmap(width, height);
}

void InvisibleSurfaceWidget::update_tracer_object_impl()
{
    do_update_tracer_object();
}

void InvisibleSurfaceWidget::do_update_tracer_object()
{
    tracer_object_ = tracer::create_invisible_surface();
}

ResourceWidget<tracer::Material> *InvisibleSurfaceWidgetCreator::create_widget(ObjectContext &obj_ctx) const
{
    return new InvisibleSurfaceWidget;
}

AGZ_EDITOR_END
