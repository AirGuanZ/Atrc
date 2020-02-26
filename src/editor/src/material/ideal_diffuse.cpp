#include <agz/editor/material/ideal_diffuse.h>
#include <agz/editor/material/material_thumbnail.h>
#include <agz/editor/ui/utility/collapsible.h>

AGZ_EDITOR_BEGIN

IdealDiffuseWidget::IdealDiffuseWidget(const InitData &clone_state, ObjectContext &obj_ctx)
    : obj_ctx_(obj_ctx)
{
    QVBoxLayout *layout             = new QVBoxLayout(this);
    Collapsible *albedo_section     = new Collapsible(this, "Albedo");
    Collapsible *normal_map_section = new Collapsible(this, "Normal");

    albedo_ = clone_state.albedo;
    if(!albedo_)
        albedo_ = new Texture2DSlot(obj_ctx_, "Constant");
    albedo_section->set_content_widget(albedo_);
    albedo_section->open();

    normal_map_ = clone_state.normal_map;
    if(!normal_map_)
        normal_map_ = new NormalMapWidget({}, obj_ctx);
    normal_map_section->set_content_widget(normal_map_);

    layout->addWidget(albedo_section);
    layout->addWidget(normal_map_section);

    setContentsMargins(0, 0, 0, 0);
    layout->setContentsMargins(0, 0, 0, 0);

    albedo_->set_dirty_callback([=]
    {
        set_dirty_flag();
    });

    connect(normal_map_, &NormalMapWidget::change_params, [=]
    {
        set_dirty_flag();
    });

    do_update_tracer_object();
}

ResourceWidget<tracer::Material> *IdealDiffuseWidget::clone()
{
    InitData clone_state;
    clone_state.albedo     = albedo_->clone();
    clone_state.normal_map = normal_map_->clone();
    return new IdealDiffuseWidget(clone_state, obj_ctx_);
}

std::unique_ptr<ResourceThumbnailProvider> IdealDiffuseWidget::get_thumbnail(int width, int height) const
{
    return std::make_unique<MaterialThumbnailProvider>(width, height, tracer_object_);
}

void IdealDiffuseWidget::update_tracer_object_impl()
{
    do_update_tracer_object();
}

void IdealDiffuseWidget::do_update_tracer_object()
{
    auto albedo_tex = albedo_->get_tracer_object();
    auto normal_map = normal_map_->get_tracer_object();
    tracer_object_ = create_ideal_diffuse(albedo_tex, std::move(normal_map));
}

ResourceWidget<tracer::Material> *IdealDiffuseWidgetCreator::create_widget(ObjectContext &obj_ctx) const
{
    return new IdealDiffuseWidget({}, obj_ctx);
}

AGZ_EDITOR_END
