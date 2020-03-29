#include <agz/editor/material/invisible_surface.h>
#include <agz/editor/material/material_thumbnail.h>
#include <agz/editor/ui/utility/collapsible.h>

AGZ_EDITOR_BEGIN

InvisibleSurfaceWidget::InvisibleSurfaceWidget(
    const InitData &init_data, ObjectContext &obj_ctx)
    : obj_ctx_(obj_ctx)
{
    bssrdf_ = init_data.bssrdf;
    if(!bssrdf_)
        bssrdf_ = new BSSRDFWidget({}, obj_ctx);

    bssrdf_->set_dirty_callback([=] { set_dirty_flag(); });

    auto layout = new QVBoxLayout(this);
    layout->addWidget(bssrdf_);

    setContentsMargins(0, 0, 0, 0);
    layout->setContentsMargins(0, 0, 0, 0);

    do_update_tracer_object();
}

ResourceWidget<tracer::Material> *InvisibleSurfaceWidget::clone()
{
    InitData init_data;
    init_data.bssrdf = bssrdf_->clone();
    return new InvisibleSurfaceWidget(init_data, obj_ctx_);
}

Box<ResourceThumbnailProvider> InvisibleSurfaceWidget::get_thumbnail(
    int width, int height) const
{
    return newBox<MaterialThumbnailProvider>(width, height, tracer_object_);
}

void InvisibleSurfaceWidget::save_asset(AssetSaver &saver)
{
    bssrdf_->save_asset(saver);
}

void InvisibleSurfaceWidget::load_asset(AssetLoader &loader)
{
    bssrdf_->load_asset(loader);

    do_update_tracer_object();
}

RC<tracer::ConfigNode> InvisibleSurfaceWidget::to_config(
    JSONExportContext &ctx) const
{
    auto grp = newRC<tracer::ConfigGroup>();
    grp->insert_str("type", "invisible_surface");
    bssrdf_->to_config(*grp, ctx);
    return grp;
}

void InvisibleSurfaceWidget::update_tracer_object_impl()
{
    do_update_tracer_object();
}

void InvisibleSurfaceWidget::do_update_tracer_object()
{
    auto bssrdf = bssrdf_->create_tracer_object({}, {});
    tracer_object_ = create_invisible_surface(bssrdf);
}

ResourceWidget<tracer::Material> *InvisibleSurfaceWidgetCreator::create_widget(
    ObjectContext &obj_ctx) const
{
    return new InvisibleSurfaceWidget({}, obj_ctx);
}

AGZ_EDITOR_END
