#include <agz/editor/entity/geometric_entity.h>

#include <agz/editor/entity/entity_factory.h>

AGZ_EDITOR_BEGIN

void init_entity_factory(EntityWidgetFactory &factory)
{
    factory.add_creator(std::make_unique<GeometricEntityWidgetCreator>());
}

AGZ_EDITOR_END
