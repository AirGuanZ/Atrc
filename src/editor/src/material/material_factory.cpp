#include <agz/editor/material/disney.h>
#include <agz/editor/material/glass.h>
#include <agz/editor/material/ideal_diffuse.h>
#include <agz/editor/material/invisible_surface.h>
#include <agz/editor/material/metal.h>
#include <agz/editor/material/mirror.h>
#include <agz/editor/material/phong.h>

#include <agz/editor/material/material_factory.h>

AGZ_EDITOR_BEGIN

void init_material_factory(MaterialWidgetFactory &factory)
{
    factory.add_creator(newBox<DisneyWidgetCreator>());
    factory.add_creator(newBox<GlassWidgetCreator>());
    factory.add_creator(newBox<IdealDiffuseWidgetCreator>());
    factory.add_creator(newBox<InvisibleSurfaceWidgetCreator>());
    factory.add_creator(newBox<MetalWidgetCreator>());
    factory.add_creator(newBox<MirrorWidgetCreator>());
    factory.add_creator(newBox<PhongWidgetCreator>());
}

AGZ_EDITOR_END
