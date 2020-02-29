#include <QCheckBox>

#include <agz/editor/material/mirror.h>
#include <agz/editor/material/material_thumbnail.h>
#include <agz/editor/texture2d/constant2d.h>
#include <agz/editor/texture2d/range.h>
#include <agz/editor/ui/utility/collapsible.h>

AGZ_EDITOR_BEGIN

MirrorWidget::MirrorWidget(const InitData &init_data, ObjectContext &obj_ctx)
    : obj_ctx_(obj_ctx)
{
    color_map_ = init_data.color_map;
    eta_       = init_data.eta;
    k_         = init_data.k;

    if(!color_map_)
        color_map_ = new Texture2DSlot(obj_ctx, "Constant");

    if(!eta_)
    {
        auto tex = new RangeWidget({ real(1.01), real(3), real(1.4) });
        eta_ = new Texture2DSlot(obj_ctx, "Range", tex, "Range");
    }

    if(!k_)
    {
        auto tex = new Constant2DWidget({
                true, Spectrum(1), Spectrum(real(3.9)) });
        k_ = new Texture2DSlot(obj_ctx, "Range", tex, "Constant");
    }

    Collapsible *color_map_section = new Collapsible(this, "Color Map");
    Collapsible *ior_section       = new Collapsible(this, "eta");
    Collapsible *k_section         = new Collapsible(this, "k");

    color_map_section->set_content_widget(color_map_);
    ior_section      ->set_content_widget(eta_);
    k_section        ->set_content_widget(k_);

    color_map_section->open();
    ior_section      ->open();
    k_section        ->open();

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(color_map_section);
    layout->addWidget(ior_section);
    layout->addWidget(k_section);
    
    setContentsMargins(0, 0, 0, 0);
    layout->setContentsMargins(0, 0, 0, 0);

    color_map_->set_dirty_callback([=] { set_dirty_flag(); });
    eta_      ->set_dirty_callback([=] { set_dirty_flag(); });
    k_        ->set_dirty_callback([=] { set_dirty_flag(); });

    do_update_tracer_object();
}

ResourceWidget<tracer::Material> *MirrorWidget::clone()
{
    InitData init_data;
    init_data.color_map = color_map_->clone();
    init_data.eta       = eta_->clone();
    init_data.k         = k_->clone();

    return new MirrorWidget(init_data, obj_ctx_);
}

std::unique_ptr<ResourceThumbnailProvider> MirrorWidget::get_thumbnail(int width, int height) const
{
    return std::make_unique<MaterialThumbnailProvider>(width, height, tracer_object_);
}

void MirrorWidget::save_asset(AssetSaver &saver)
{
    color_map_->save_asset(saver);
    eta_      ->save_asset(saver);
    k_        ->save_asset(saver);
}

void MirrorWidget::load_asset(AssetLoader &loader)
{
    color_map_->load_asset(loader);
    eta_      ->load_asset(loader);
    k_        ->load_asset(loader);

    do_update_tracer_object();
}

void MirrorWidget::update_tracer_object_impl()
{
    do_update_tracer_object();
}

void MirrorWidget::do_update_tracer_object()
{
    auto color_map = color_map_->get_tracer_object();
    auto ior       = eta_      ->get_tracer_object();
    auto k         = k_->get_tracer_object();
    tracer_object_ = create_mirror(color_map, ior, k);
}

ResourceWidget<tracer::Material> *MirrorWidgetCreator::create_widget(ObjectContext &obj_ctx) const
{
    return new MirrorWidget({}, obj_ctx);
}

AGZ_EDITOR_END
