#include <agz/editor/material/invisible_surface.h>
#include <agz/editor/material/material_thumbnail.h>

AGZ_EDITOR_BEGIN

InvisibleSurfaceWidget::InvisibleSurfaceWidget()
{
    do_update_tracer_object();
}

ResourceWidget<tracer::Material> *InvisibleSurfaceWidget::clone()
{
    return new InvisibleSurfaceWidget;
}

std::unique_ptr<ResourceThumbnailProvider> InvisibleSurfaceWidget::get_thumbnail(int width, int height) const
{
    return std::make_unique<MaterialThumbnailProvider>(width, height, tracer_object_);
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
