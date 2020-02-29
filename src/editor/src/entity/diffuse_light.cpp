#include <agz/editor/entity/diffuse_light.h>
#include <agz/editor/ui/utility/collapsible.h>

AGZ_EDITOR_BEGIN

DiffuseLightEntityWidget::DiffuseLightEntityWidget(const CloneState &clone_state, ObjectContext &obj_ctx)
    : obj_ctx_(obj_ctx)
{
    geometry_ = clone_state.geometry;
    if(!geometry_)
        geometry_ = new GeometrySlot(obj_ctx_, "Triangle Mesh");

    radiance_ = new SpectrumInput(this);
    radiance_->set_value(clone_state.radiance);

    transform_ = new Transform3DWidget(clone_state.transform);

    Collapsible *geometry_section  = new Collapsible(this, "Geometry");
    Collapsible *radiance_section  = new Collapsible(this, "Radiance");
    Collapsible *transform_section = new Collapsible(this, "Transform");

    geometry_section ->set_content_widget(geometry_);
    radiance_section ->set_content_widget(radiance_);
    transform_section->set_content_widget(transform_);

    geometry_section ->open();
    radiance_section ->open();
    transform_section->open();

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(geometry_section);
    layout->addWidget(transform_section);
    layout->addWidget(radiance_section);

    setContentsMargins(0, 0, 0, 0);
    layout->setContentsMargins(0, 0, 0, 0);

    geometry_->set_dirty_callback([=]
    {
        set_dirty_flag();
    });

    connect(radiance_, &SpectrumInput::edit_value, [=](const Spectrum&)
    {
        set_dirty_flag();
    });

    connect(transform_, &Transform3DWidget::edit_transform, [=]
    {
        set_entity_transform_dirty();
        set_dirty_flag();
    });

    geometry_->set_geometry_vertices_dirty_callback([=]
    {
        set_geometry_vertices_dirty();
    });

    do_update_tracer_object();
}

ResourceWidget<tracer::Entity> *DiffuseLightEntityWidget::clone()
{
    CloneState clone_state;
    clone_state.geometry  = geometry_->clone();
    clone_state.radiance  = radiance_->get_value();
    clone_state.transform = transform_->get_transform();
    return new DiffuseLightEntityWidget(clone_state, obj_ctx_);
}

void DiffuseLightEntityWidget::save_asset(AssetSaver &saver)
{
    geometry_->save_asset(saver);
    saver.write(radiance_->get_value());
    saver.write(transform_->get_transform());
}

void DiffuseLightEntityWidget::load_asset(AssetLoader &loader)
{
    geometry_->load_asset(loader);
    radiance_->set_value(loader.read<Spectrum>());
    transform_->set_transform(loader.read<DirectTransform>());

    do_update_tracer_object();
}

std::vector<EntityInterface::Vertex> DiffuseLightEntityWidget::get_vertices() const
{
    return geometry_->get_vertices();
}

DirectTransform DiffuseLightEntityWidget::get_transform() const
{
    return transform_->get_transform();
}

void DiffuseLightEntityWidget::set_transform(const DirectTransform &transform)
{
    transform_->set_transform(transform);
    set_entity_transform_dirty();
    set_dirty_flag();
}

void DiffuseLightEntityWidget::update_tracer_object_impl()
{
    do_update_tracer_object();
}

void DiffuseLightEntityWidget::do_update_tracer_object()
{
    auto geometry = geometry_->get_tracer_object();
    auto radiance = radiance_->get_value();

    tracer::MediumInterface med;
    med.in  = tracer::create_void();
    med.out = tracer::create_void();

    auto geometry_wrapper = create_transform_wrapper(
        geometry, tracer::Transform3(get_transform().compose()));

    tracer_object_ = create_diffuse_light(geometry_wrapper, radiance, med, false);
}

ResourceWidget<tracer::Entity> *DiffuseLightEntityWidgetCreator::create_widget(ObjectContext &obj_ctx) const
{
    return new DiffuseLightEntityWidget({}, obj_ctx);
}

AGZ_EDITOR_END
