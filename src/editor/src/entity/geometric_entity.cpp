#include <agz/editor/entity/geometric_entity.h>
#include <agz/editor/ui/utility/collapsible.h>

AGZ_EDITOR_BEGIN

GeometricEntityWidget::GeometricEntityWidget(const CloneState &clone_state, ObjectContext &obj_ctx)
    : obj_ctx_(obj_ctx)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    Collapsible *geometry_section  = new Collapsible(this, "Geometry");
    Collapsible *transform_section = new Collapsible(this, "Transform");
    Collapsible *material_section  = new Collapsible(this, "Material");

    geometry_  = clone_state.geometry;
    material_  = clone_state.material;
    transform_ = new Transform3DWidget(clone_state.transform);

    if(!geometry_)
        geometry_ = new GeometrySlot(obj_ctx_, "Triangle Mesh");

    if(!material_)
        material_ = new MaterialSlot(obj_ctx_, "Ideal Diffuse");

    geometry_->set_dirty_callback([=]
    {
        set_dirty_flag();
    });

    material_->set_dirty_callback([=] {
        set_dirty_flag();
    });

    connect(transform_, &Transform3DWidget::edit_transform, [=]
    {
        set_entity_transform_dirty();
        set_dirty_flag();
    });

    geometry_section ->set_content_widget(geometry_);
    transform_section->set_content_widget(transform_);
    material_section ->set_content_widget(material_);

    geometry_section ->open();
    transform_section->open();
    material_section ->open();

    layout->setAlignment(Qt::AlignTop);
    layout->addWidget(geometry_section);
    layout->addWidget(transform_section);
    layout->addWidget(material_section);

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
    clone_state.geometry  = geometry_->clone();
    clone_state.material  = material_->clone();
    clone_state.transform = transform_->get_transform();
    return new GeometricEntityWidget(clone_state, obj_ctx_);
}

QPixmap GeometricEntityWidget::get_thumbnail(int width, int height) const
{
    return QPixmap(width, height);
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
    auto geometry = geometry_->update_tracer_object();
    auto material = material_->update_tracer_object();

    tracer::MediumInterface med;
    med.in  = tracer::create_void();
    med.out = tracer::create_void();

    auto geometry_wrapper = create_transform_wrapper(geometry, tracer::Transform3(get_transform().compose()));

    tracer_object_ = create_geometric(geometry_wrapper, material, med, false);
}

ResourceWidget<tracer::Entity> *GeometricEntityWidgetCreator::create_widget(ObjectContext &obj_ctx) const
{
    return new GeometricEntityWidget({}, obj_ctx);
}

AGZ_EDITOR_END
