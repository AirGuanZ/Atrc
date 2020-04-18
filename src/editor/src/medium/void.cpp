#include <agz/editor/medium/void.h>
#include <agz/tracer/create/medium.h>

AGZ_EDITOR_BEGIN

VoidWidget::VoidWidget()
{
    tracer_object_ = tracer::create_void();
}

ResourceWidget<tracer::Medium> *VoidWidget::clone()
{
    return new VoidWidget;
}

RC<tracer::ConfigNode> VoidWidget::to_config(JSONExportContext &ctx) const
{
    auto grp = newRC<tracer::ConfigGroup>();
    grp->insert_str("type", "void");
    return grp;
}

void VoidWidget::update_tracer_object_impl()
{
    // do nothing
}

ResourceWidget<tracer::Medium> *VoidWidgetCreator::create_widget(
    ObjectContext &obj_ctx) const
{
    return new VoidWidget;
}

AGZ_EDITOR_END
