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

std::shared_ptr<tracer::ConfigNode> VoidWidget::to_config(JSONExportContext &ctx) const
{
    auto grp = std::make_shared<tracer::ConfigGroup>();
    grp->insert_str("type", "void");
    return grp;
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
