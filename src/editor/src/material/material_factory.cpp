#include <agz/editor/material/disney.h>
#include <agz/editor/material/glass.h>
#include <agz/editor/material/ideal_diffuse.h>
#include <agz/editor/material/invisible_surface.h>
#include <agz/editor/material/mirror.h>
#include <agz/editor/material/phong.h>

#include <agz/editor/material/material_factory.h>

AGZ_EDITOR_BEGIN

void init_material_factory(MaterialWidgetFactory &factory)
{
    factory.add_creator(std::make_unique<DisneyWidgetCreator>());
    factory.add_creator(std::make_unique<GlassWidgetCreator>());
    factory.add_creator(std::make_unique<IdealDiffuseWidgetCreator>());
    factory.add_creator(std::make_unique<InvisibleSurfaceWidgetCreator>());
    factory.add_creator(std::make_unique<MirrorWidgetCreator>());
    factory.add_creator(std::make_unique<PhongWidgetCreator>());
}

AGZ_EDITOR_END
