#include <QVBoxLayout>

#include <agz/editor/material/phong.h>
#include <agz/editor/material/material_thumbnail.h>
#include <agz/editor/texture2d/range.h>
#include <agz/editor/ui/utility/collapsible.h>
#include <agz/tracer/create/material.h>

AGZ_EDITOR_BEGIN

PhongWidget::PhongWidget(const InitData &init_data, ObjectContext &obj_ctx)
    : obj_ctx_(obj_ctx)
{
    d_          = init_data.d;
    s_          = init_data.s;
    ns_         = init_data.ns;
    normal_map_ = init_data.normal_map;

    if(!d_)
        d_ = new Texture2DSlot(obj_ctx_, "Constant");

    if(!s_)
        s_ = new Texture2DSlot(obj_ctx_, "Constant");

    if(!ns_)
    {
        auto tex = new RangeWidget({ 0, 5000, 2000 });
        ns_ = new Texture2DSlot(obj_ctx_, "Constant", tex, "Range");
    }

    if(!normal_map_)
        normal_map_ = new NormalMapWidget({}, obj_ctx_);

    Collapsible *d_sec   = new Collapsible(this, "D");
    Collapsible *s_sec   = new Collapsible(this, "S");
    Collapsible *ns_sec  = new Collapsible(this, "NS");
    Collapsible *nor_sec = new Collapsible(this, "Normal Map");

    d_sec  ->set_content_widget(d_);
    s_sec  ->set_content_widget(s_);
    ns_sec ->set_content_widget(ns_);
    nor_sec->set_content_widget(normal_map_);

    d_sec ->open();
    s_sec ->open();
    ns_sec->open();

    d_ ->set_dirty_callback([=] { set_dirty_flag(); });
    s_ ->set_dirty_callback([=] { set_dirty_flag(); });
    ns_->set_dirty_callback([=] { set_dirty_flag(); });

    connect(normal_map_, &NormalMapWidget::change_params, [=]
    {
        set_dirty_flag();
    });

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(d_sec);
    layout->addWidget(s_sec);
    layout->addWidget(ns_sec);
    layout->addWidget(nor_sec);

    setContentsMargins(0, 0, 0, 0);
    layout->setContentsMargins(0, 0, 0, 0);

    do_update_tracer_object();
}

ResourceWidget<tracer::Material> *PhongWidget::clone()
{
    InitData init_data;
    init_data.d          = d_->clone();
    init_data.s          = s_->clone();
    init_data.ns         = ns_->clone();
    init_data.normal_map = normal_map_->clone();
    return new PhongWidget(init_data, obj_ctx_);
}

Box<ResourceThumbnailProvider> PhongWidget::get_thumbnail(
    int width, int height) const
{
    return newBox<MaterialThumbnailProvider>(width, height, tracer_object_);
}

void PhongWidget::save_asset(AssetSaver &saver)
{
    d_         ->save_asset(saver);
    s_         ->save_asset(saver);
    ns_        ->save_asset(saver);
    normal_map_->save_asset(saver);
}

void PhongWidget::load_asset(AssetLoader &loader)
{
    d_         ->load_asset(loader);
    s_         ->load_asset(loader);
    ns_        ->load_asset(loader);
    normal_map_->load_asset(loader);

    do_update_tracer_object();
}

RC<tracer::ConfigNode> PhongWidget::to_config(JSONExportContext &ctx) const
{
    auto grp = newRC<tracer::ConfigGroup>();
    grp->insert_str("type", "phong");

    grp->insert_child("d", d_->to_config(ctx));
    grp->insert_child("s", s_->to_config(ctx));
    grp->insert_child("ns", ns_->to_config(ctx));

    if(normal_map_->is_enabled())
        grp->insert_child("normal_map", normal_map_->to_config(ctx));

    return grp;
}

void PhongWidget::update_tracer_object_impl()
{
    do_update_tracer_object();
}

void PhongWidget::do_update_tracer_object()
{
    auto d   = d_->get_tracer_object();
    auto s   = s_->get_tracer_object();
    auto ns  = ns_->get_tracer_object();
    auto nor = normal_map_->get_tracer_object();

    tracer_object_ = create_phong(
        std::move(d), std::move(s), std::move(ns), std::move(nor));
}

ResourceWidget<tracer::Material> *PhongWidgetCreator::create_widget(
    ObjectContext &obj_ctx) const
{
    return new PhongWidget({}, obj_ctx);
}

AGZ_EDITOR_END
