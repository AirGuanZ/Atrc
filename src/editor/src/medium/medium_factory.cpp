#include <agz/editor/medium/heterogeneous.h>
#include <agz/editor/medium/homogeneous.h>
#include <agz/editor/medium/void.h>

#include <agz/editor/medium/medium_factory.h>

AGZ_EDITOR_BEGIN

void init_medium_factory(MediumWidgetFactory &factory)
{
    factory.add_creator(std::make_unique<HeterogeneousWidgetCreator>());
    factory.add_creator(std::make_unique<HomogeneousWidgetCreator>());
    factory.add_creator(std::make_unique<VoidWidgetCreator>());
}

AGZ_EDITOR_END
