#include <agz/editor/geometry/sphere.h>
#include <agz/editor/geometry/triangle_bvh.h>

#include <agz/editor/geometry/geometry_factory.h>

AGZ_EDITOR_BEGIN

void init_geometry_factory(GeometryWidgetFactory &factory)
{
    factory.add_creator(std::make_unique<SphereWidgetCreator>());
    factory.add_creator(std::make_unique<TriangleBVHWidgetCreator>());
}

AGZ_EDITOR_END
