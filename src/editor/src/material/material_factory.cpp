#include <agz/editor/material/ideal_diffuse.h>
#include <agz/editor/material/disney.h>

#include <agz/editor/material/material_factory.h>

AGZ_EDITOR_BEGIN

void init_material_factory(MaterialWidgetFactory &factory)
{
    factory.add_creator(std::make_unique<IdealDiffuseWidgetCreator>());
    factory.add_creator(std::make_unique<DisneyWidgetCreator>());
}

AGZ_EDITOR_END
