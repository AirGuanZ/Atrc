#include <QGridLayout>

#include <agz/editor/entity/geometric_entity.h>
#include <agz/editor/imexport/json_export_context.h>
#include <agz/editor/ui/utility/collapsible.h>

AGZ_EDITOR_BEGIN

GeometricEntityWidget::GeometricEntityWidget(
    const CloneState &clone_state, ObjectContext &obj_ctx)
    : obj_ctx_(obj_ctx)
{
    QGridLayout *layout = new QGridLayout(this);
    Collapsible *geometry_section   = new Collapsible(this, "Geometry");
    Collapsible *transform_section  = new Collapsible(this, "Transform");
    Collapsible *material_section   = new Collapsible(this, "Material");
    Collapsible *medium_in_section  = new Collapsible(this, "MediumIn");
    Collapsible *medium_out_section = new Collapsible(this, "MediumOut");

    geometry_      = clone_state.geometry;
    material_      = clone_state.material;
    medium_in_     = clone_state.medium_in;
    medium_out_    = clone_state.medium_out;
    emit_radiance_ = new SpectrumInput;
    transform_     = new Transform3DWidget(clone_state.transform);

    if(!geometry_)
        geometry_ = new GeometrySlot(obj_ctx_, "Triangle Mesh");

    if(!material_)
        material_ = new MaterialSlot(obj_ctx_, "Ideal Diffuse");

    if(!medium_in_)
        medium_in_ = new MediumSlot(obj_ctx_, "Void");

    if(!medium_out_)
        medium_out_ = new MediumSlot(obj_ctx_, "Void");

    emit_radiance_->set_value(clone_state.emit_radiance);

    geometry_->set_dirty_callback([=]
    {
        set_dirty_flag();
    });

    material_->set_dirty_callback([=]
    {
        set_dirty_flag();
    });

    medium_in_->set_dirty_callback([=]
    {
        set_dirty_flag();
    });

    medium_out_->set_dirty_callback([=]
    {
        set_dirty_flag();
    });

    connect(emit_radiance_, &SpectrumInput::edit_value, [=](const Spectrum &)
    {
        set_dirty_flag();
    });

    connect(transform_, &Transform3DWidget::edit_transform, [=]
    {
        set_entity_transform_dirty();
        set_dirty_flag();
    });

    geometry_section  ->set_content_widget(geometry_);
    transform_section ->set_content_widget(transform_);
    material_section  ->set_content_widget(material_);
    medium_in_section ->set_content_widget(medium_in_);
    medium_out_section->set_content_widget(medium_out_);

    geometry_section ->open();
    transform_section->open();
    material_section ->open();

    layout->addWidget(geometry_section, 0, 0, 1, 2);
    layout->addWidget(transform_section, 1, 0, 1, 2);
    layout->addWidget(new QLabel("   Emit Radiance", this), 2, 0, 1, 1);
    layout->addWidget(emit_radiance_, 2, 1, 1, 1);
    layout->addWidget(material_section, 3, 0, 1, 2);
    layout->addWidget(medium_in_section, 4, 0, 1, 2);
    layout->addWidget(medium_out_section, 5, 0, 1, 2);

    setContentsMargins(0, 0, 0, 0);
    layout->setContentsMargins(0, 0, 0, 0);

    geometry_->set_geometry_vertices_dirty_callback([=]
    {
        set_geometry_vertices_dirty();
    });

    do_update_tracer_object();
}

ResourceWidget<tracer::Entity> *GeometricEntityWidget::clone()
{
    CloneState clone_state;
    clone_state.geometry      = geometry_->clone();
    clone_state.material      = material_->clone();
    clone_state.emit_radiance = emit_radiance_->get_value();
    clone_state.transform     = transform_->get_transform();
    return new GeometricEntityWidget(clone_state, obj_ctx_);
}

void GeometricEntityWidget::save_asset(AssetSaver &saver)
{
    geometry_->save_asset(saver);
    material_->save_asset(saver);
    medium_in_->save_asset(saver);
    medium_out_->save_asset(saver);
    saver.write(emit_radiance_->get_value());
    saver.write(transform_->get_transform());
}

void GeometricEntityWidget::load_asset(AssetLoader &loader)
{
    geometry_->load_asset(loader);
    material_->load_asset(loader);
    medium_in_->load_asset(loader);
    medium_out_->load_asset(loader);
    emit_radiance_->set_value(loader.read<Spectrum>());
    transform_->set_transform(loader.read<DirectTransform>());

    do_update_tracer_object();
}

RC<tracer::ConfigNode> GeometricEntityWidget::to_config(
    JSONExportContext &ctx) const
{
    auto grp = newRC<tracer::ConfigGroup>();
    grp->insert_str("type", "geometric");

    auto geo_grp = newRC<tracer::ConfigGroup>();
    geo_grp->insert_str("type", "transform_wrapper");
    geo_grp->insert_child("internal", geometry_->to_config(ctx));
    geo_grp->insert_child("transform", transform_->get_transform().to_config());

    grp->insert_child("geometry", geo_grp);
    grp->insert_child("material", material_->to_config(ctx));
    grp->insert_child("med_in",   medium_in_->to_config(ctx));
    grp->insert_child("med_out",  medium_out_->to_config(ctx));
    grp->insert_child(
        "emit_radiance", tracer::ConfigArray::from_spectrum(emit_radiance_->get_value()));

    return grp;
}

std::vector<EntityInterface::Vertex> GeometricEntityWidget::get_vertices() const
{
    return geometry_->get_vertices();
}

DirectTransform GeometricEntityWidget::get_transform() const
{
    return transform_->get_transform();
}

void GeometricEntityWidget::set_transform(const DirectTransform &transform)
{
    transform_->set_transform(transform);
    set_entity_transform_dirty();
    set_dirty_flag();
}

void GeometricEntityWidget::update_tracer_object_impl()
{
    do_update_tracer_object();
}

void GeometricEntityWidget::do_update_tracer_object()
{
    auto geometry = geometry_->get_tracer_object();
    auto material = material_->get_tracer_object();

    tracer::MediumInterface med;
    med.in  = medium_in_ ->get_tracer_object();
    med.out = medium_out_->get_tracer_object();

    const Spectrum emit_radiance = emit_radiance_->get_value();

    auto geometry_wrapper = create_transform_wrapper(
        geometry, tracer::Transform3(get_transform().compose()));

    tracer_object_ = create_geometric(
        geometry_wrapper, material, med, emit_radiance, false);
}

ResourceWidget<tracer::Entity> *GeometricEntityWidgetCreator::create_widget(
    ObjectContext &obj_ctx) const
{
    return new GeometricEntityWidget({}, obj_ctx);
}

AGZ_EDITOR_END
