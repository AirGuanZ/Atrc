#include <agz/editor/material/disney.h>
#include <agz/editor/material/material_thumbnail.h>
#include <agz/editor/texture2d/range.h>
#include <agz/editor/ui/utility/collapsible.h>

AGZ_EDITOR_BEGIN

DisneyWidget::DisneyWidget(const InitData &init_data, ObjectContext &obj_ctx)
    : obj_ctx_(obj_ctx)
{
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

    base_color_ = init_data.base_color;
    if(!base_color_)
        base_color_ = new Texture2DSlot(obj_ctx_, "Constant");
    add_section("Color", base_color_, true);
    base_color_->set_dirty_callback([=] { set_dirty_flag(); });

    metallic_ = init_data.metallic;
    if(!metallic_)
        metallic_ = new Texture2DSlot(obj_ctx_, "Range");
    add_section("Metallic", metallic_, true);
    metallic_->set_dirty_callback([=] { set_dirty_flag(); });

    roughness_ = init_data.roughness;
    if(!roughness_)
        roughness_ = new Texture2DSlot(obj_ctx_, "Range");
    add_section("Roughness", roughness_, true);
    roughness_->set_dirty_callback([=] { set_dirty_flag(); });

    transmission_ = init_data.transmission;
    if(!transmission_)
        transmission_ = new Texture2DSlot(obj_ctx_, "Range");
    add_section("Transmission", transmission_);
    transmission_->set_dirty_callback([=] { set_dirty_flag(); });

    transmission_roughness_ = init_data.transmission_roughness;
    if(!transmission_roughness_)
        transmission_roughness_ = new Texture2DSlot(obj_ctx_, "Range");
    add_section("Transmission Roughness", transmission_roughness_);
    transmission_roughness_->set_dirty_callback([=] { set_dirty_flag(); });

    ior_ = init_data.ior;
    if(!ior_)
    {
        ior_ = new Texture2DSlot(
            obj_ctx_, "Range",
            new RangeWidget({ real(1.01), 3, real(1.5) }), "Range");
    }
    add_section("IOR", ior_);
    ior_->set_dirty_callback([=] { set_dirty_flag(); });

    specular_scale_ = init_data.specular_scale;
    if(!specular_scale_)
    {
        specular_scale_ = new Texture2DSlot(
            obj_ctx_, "Range",
            new RangeWidget({ 0, 1, 1 }), "Range");
    }
    add_section("Specular Scale", specular_scale_);
    specular_scale_->set_dirty_callback([=] { set_dirty_flag(); });

    specular_tint_ = init_data.specular_tint;
    if(!specular_tint_)
        specular_tint_ = new Texture2DSlot(obj_ctx_, "Range");
    add_section("Specular Tint", specular_tint_);
    specular_tint_->set_dirty_callback([=] { set_dirty_flag(); });

    anisotropic_ = init_data.anisotropic;
    if(!anisotropic_)
        anisotropic_ = new Texture2DSlot(obj_ctx_, "Range");
    add_section("Anisotropic", anisotropic_);
    anisotropic_->set_dirty_callback([=] { set_dirty_flag(); });

    sheen_ = init_data.sheen;
    if(!sheen_)
        sheen_ = new Texture2DSlot(obj_ctx_, "Range");
    add_section("Sheen", sheen_);
    sheen_->set_dirty_callback([=] { set_dirty_flag(); });

    sheen_tint_ = init_data.sheen_tint;
    if(!sheen_tint_)
        sheen_tint_ = new Texture2DSlot(obj_ctx_, "Range");
    add_section("Sheen Tint", sheen_tint_);
    sheen_tint_->set_dirty_callback([=] { set_dirty_flag(); });

    clearcoat_ = init_data.clearcoat;
    if(!clearcoat_)
        clearcoat_ = new Texture2DSlot(obj_ctx_, "Range");
    add_section("Clearcoat", clearcoat_);
    clearcoat_->set_dirty_callback([=] { set_dirty_flag(); });

    clearcoat_gloss_ = init_data.clearcoat_gloss;
    if(!clearcoat_gloss_)
        clearcoat_gloss_ = new Texture2DSlot(obj_ctx_, "Range");
    add_section("Clearcoat Gloss", clearcoat_gloss_);
    clearcoat_gloss_->set_dirty_callback([=] { set_dirty_flag(); });

    normal_map_ = init_data.normal_map;
    if(!normal_map_)
        normal_map_ = new NormalMapWidget({}, obj_ctx_);
    add_section("Normal Map", normal_map_);

    setContentsMargins(0, 0, 0, 0);
    layout->setContentsMargins(0, 0, 0, 0);
    
    connect(normal_map_, &NormalMapWidget::change_params, [=]
    {
        set_dirty_flag();
    });

    do_update_tracer_object();
}

ResourceWidget<tracer::Material> *DisneyWidget::clone()
{
    InitData init_data;
    init_data.base_color             = base_color_            ->clone();
    init_data.metallic               = metallic_              ->clone();
    init_data.roughness              = roughness_             ->clone();
    init_data.transmission           = transmission_          ->clone();
    init_data.transmission_roughness = transmission_roughness_->clone();
    init_data.ior                    = ior_                   ->clone();
    init_data.specular_scale         = specular_scale_        ->clone();
    init_data.specular_tint          = specular_tint_         ->clone();
    init_data.anisotropic            = anisotropic_           ->clone();
    init_data.sheen                  = sheen_                 ->clone();
    init_data.sheen_tint             = sheen_tint_            ->clone();
    init_data.clearcoat              = clearcoat_             ->clone();
    init_data.clearcoat_gloss        = clearcoat_gloss_       ->clone();
    init_data.normal_map             = normal_map_            ->clone();
    return new DisneyWidget(init_data, obj_ctx_);
}

Box<ResourceThumbnailProvider> DisneyWidget::get_thumbnail(
    int width, int height) const
{
    return newBox<MaterialThumbnailProvider>(width, height, tracer_object_);
}

void DisneyWidget::save_asset(AssetSaver &saver)
{
    base_color_            ->save_asset(saver);
    metallic_              ->save_asset(saver);
    roughness_             ->save_asset(saver);
    transmission_          ->save_asset(saver);
    transmission_roughness_->save_asset(saver);
    ior_                   ->save_asset(saver);
    specular_scale_        ->save_asset(saver);
    specular_tint_         ->save_asset(saver);
    anisotropic_           ->save_asset(saver);
    sheen_                 ->save_asset(saver);
    sheen_tint_            ->save_asset(saver);
    clearcoat_             ->save_asset(saver);
    clearcoat_gloss_       ->save_asset(saver);
    normal_map_            ->save_asset(saver);
}

void DisneyWidget::load_asset(AssetLoader &loader)
{
    base_color_            ->load_asset(loader);
    metallic_              ->load_asset(loader);
    roughness_             ->load_asset(loader);
    transmission_          ->load_asset(loader);
    transmission_roughness_->load_asset(loader);
    ior_                   ->load_asset(loader);
    specular_scale_        ->load_asset(loader);
    specular_tint_         ->load_asset(loader);
    anisotropic_           ->load_asset(loader);
    sheen_                 ->load_asset(loader);
    sheen_tint_            ->load_asset(loader);
    clearcoat_             ->load_asset(loader);
    clearcoat_gloss_       ->load_asset(loader);
    normal_map_            ->load_asset(loader);

    do_update_tracer_object();
}

RC<tracer::ConfigNode> DisneyWidget::to_config(JSONExportContext &ctx) const
{
    auto grp = newRC<tracer::ConfigGroup>();
    grp->insert_str("type", "disney");

    grp->insert_child("base_color",             base_color_->to_config(ctx));
    grp->insert_child("metallic",               metallic_->to_config(ctx));
    grp->insert_child("roughness",              roughness_->to_config(ctx));
    grp->insert_child("transmission",           transmission_->to_config(ctx));
    grp->insert_child("transmission_roughness", transmission_roughness_->to_config(ctx));
    grp->insert_child("ior",                    ior_->to_config(ctx));
    grp->insert_child("specular_scale",         specular_scale_->to_config(ctx));
    grp->insert_child("specular_tint",          specular_tint_->to_config(ctx));
    grp->insert_child("anisotropic",            anisotropic_->to_config(ctx));
    grp->insert_child("sheen",                  sheen_->to_config(ctx));
    grp->insert_child("sheen_tint",             sheen_tint_->to_config(ctx));
    grp->insert_child("clearcoat",              clearcoat_->to_config(ctx));
    grp->insert_child("clearcoat_gloss",        clearcoat_gloss_->to_config(ctx));

    if(normal_map_->is_enabled())
        grp->insert_child("normal_map", normal_map_->to_config(ctx));

    return grp;
}

void DisneyWidget::update_tracer_object_impl()
{
    do_update_tracer_object();
}

void DisneyWidget::do_update_tracer_object()
{
    auto base_color             = base_color_            ->get_tracer_object();
    auto metallic               = metallic_              ->get_tracer_object();
    auto roughness              = roughness_             ->get_tracer_object();
    auto transmission           = transmission_          ->get_tracer_object();
    auto transmission_roughness = transmission_roughness_->get_tracer_object();
    auto ior                    = ior_                   ->get_tracer_object();
    auto specular_scale         = specular_scale_        ->get_tracer_object();
    auto specular_tint          = specular_tint_         ->get_tracer_object();
    auto anisotropic            = anisotropic_           ->get_tracer_object();
    auto sheen                  = sheen_                 ->get_tracer_object();
    auto sheen_tint             = sheen_tint_            ->get_tracer_object();
    auto clearcoat              = clearcoat_             ->get_tracer_object();
    auto clearcoat_gloss        = clearcoat_gloss_       ->get_tracer_object();
    auto normal_map             = normal_map_            ->get_tracer_object();

    tracer_object_ = create_disney(
        base_color, metallic, roughness, transmission, transmission_roughness,
        ior, specular_scale, specular_tint, anisotropic, sheen, sheen_tint,
        clearcoat, clearcoat_gloss, std::move(normal_map));
}

ResourceWidget<tracer::Material> *DisneyWidgetCreator::create_widget(
    ObjectContext &obj_ctx) const
{
    return new DisneyWidget({}, obj_ctx);
}

AGZ_EDITOR_END
