#include <agz/editor/medium/void.h>

AGZ_EDITOR_BEGIN

VoidWidget::VoidWidget()
{
    tracer_object_ = tracer::create_void();
}

ResourceWidget<tracer::Medium> *VoidWidget::clone()
{
    return new VoidWidget;
}

void VoidWidget::update_tracer_object_impl()
{
    // do nothing
}

ResourceWidget<tracer::Medium> *VoidWidgetCreator::create_widget(ObjectContext &obj_ctx) const
{
    return new VoidWidget;
}

AGZ_EDITOR_END
