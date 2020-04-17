#include <agz/editor/material/fabric.h>
#include <agz/editor/material/material_thumbnail.h>
#include <agz/editor/texture2d/range.h>
#include <agz/editor/ui/utility/collapsible.h>

AGZ_EDITOR_BEGIN

FabricWidget::FabricWidget(
    const InitData &init_data, ObjectContext &obj_ctx)
    : obj_ctx_(obj_ctx)
{
    color_ = init_data.color;
    if(!color_)
        color_ = new Texture2DSlot(obj_ctx_, "Constant");
    color_->set_dirty_callback([=] { set_dirty_flag(); });

    roughness_ = init_data.roughness;
    if(!roughness_)
    {
        auto w = new RangeWidget({ real(0), real(1), real(0.5) });
        roughness_ = new Texture2DSlot(obj_ctx_, "Range", w, "Range");
    }
    roughness_->set_dirty_callback([=] { set_dirty_flag(); });

    normal_map_ = init_data.normal_map;
    if(!normal_map_)
        normal_map_ = new NormalMapWidget({}, obj_ctx_);
    connect(normal_map_, &NormalMapWidget::change_params, [=]
    {
        set_dirty_flag();
    });
    QVBoxLayout *layout = new QVBoxLayout(this);

    auto add_section = [=](
        const QString &title, QWidget *widget, bool open = false)
    {
        Collapsible *section = new Collapsible(this, title);
        section->set_content_widget(widget);
        layout->addWidget(section);
        if(open)
            section->open();
    };

    add_section("Color", color_, true);
    add_section("Roughness", roughness_, true);
    add_section("Normal Map", normal_map_, false);

    setContentsMargins(0, 0, 0, 0);
    layout->setContentsMargins(0, 0, 0, 0);

    do_update_tracer_object();
}

ResourceWidget<tracer::Material> *FabricWidget::clone()
{
    InitData init_data;
    init_data.color      = color_->clone();
    init_data.roughness  = roughness_->clone();
    init_data.normal_map = normal_map_->clone();
    return new FabricWidget(init_data, obj_ctx_);
}

Box<ResourceThumbnailProvider> FabricWidget::get_thumbnail(
    int width, int height) const
{
    return newBox<MaterialThumbnailProvider>(width, height, tracer_object_);
}

void FabricWidget::save_asset(AssetSaver &saver)
{
    color_->save_asset(saver);
    roughness_->save_asset(saver);
    normal_map_->save_asset(saver);
}

void FabricWidget::load_asset(AssetLoader &loader)
{
    color_->load_asset(loader);
    roughness_->load_asset(loader);
    normal_map_->load_asset(loader);

    do_update_tracer_object();
}

RC<tracer::ConfigNode> FabricWidget::to_config(JSONExportContext &ctx) const
{
    auto grp = newRC<tracer::ConfigGroup>();
    grp->insert_str("type", "dream_works_fabric");

    grp->insert_child("color", color_->to_config(ctx));
    grp->insert_child("roughness", roughness_->to_config(ctx));
    
    if(normal_map_->is_enabled())
        grp->insert_child("normal", normal_map_->to_config(ctx));

    return grp;
}

void FabricWidget::update_tracer_object_impl()
{
    do_update_tracer_object();
}

void FabricWidget::do_update_tracer_object()
{
    auto color     = color_->get_tracer_object();
    auto roughness = roughness_->get_tracer_object();
    auto normal    = normal_map_->get_tracer_object();
    tracer_object_ = create_dream_works_fabric(
        std::move(color), std::move(roughness), std::move(normal));
}

ResourceWidget<tracer::Material> *FabricWidgetCreator::create_widget(
    ObjectContext &obj_ctx) const
{
    return new FabricWidget({}, obj_ctx);
}

AGZ_EDITOR_END
