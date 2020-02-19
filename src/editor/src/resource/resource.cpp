#include <agz/editor/entity/entity_factory.h>
#include <agz/editor/envir_light/envir_light_factory.h>
#include <agz/editor/geometry/geometry_factory.h>
#include <agz/editor/material/material_factory.h>
#include <agz/editor/texture2d/texture2d_factory.h>

AGZ_EDITOR_BEGIN

ObjectContext::ObjectContext(Editor *editor)
{
    init_entity_factory     (factory<tracer::Entity>());
    init_envir_light_factory(factory<tracer::EnvirLight>());
    init_geometry_factory   (factory<tracer::Geometry>());
    init_material_factory   (factory<tracer::Material>());
    init_texture2d_factory  (factory<tracer::Texture2D>());

    std::get<std::unique_ptr<ResourcePool<tracer::Material>>>(pools_) =
        std::make_unique<MaterialPool>(*this, editor, "Ideal Diffuse");
    std::get<std::unique_ptr<ResourcePool<tracer::Texture2D>>>(pools_) =
        std::make_unique<Texture2DPool>(*this, editor, "Constant");
    std::get<std::unique_ptr<ResourcePool<tracer::Geometry>>>(pools_) =
        std::make_unique<GeometryPool>(*this, editor, "Sphere");
}

AGZ_EDITOR_END
