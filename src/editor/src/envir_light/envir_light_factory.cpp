#include <agz/editor/envir_light/envir_light_factory.h>
#include <agz/editor/envir_light/ibl.h>
#include <agz/editor/envir_light/native_sky.h>
#include <agz/editor/envir_light/no_envir_light.h>

AGZ_EDITOR_BEGIN

void init_envir_light_factory(EnvirLightWidgetFactory &factory)
{
    factory.add_creator(std::make_unique<IBLCreator>());
    factory.add_creator(std::make_unique<NativeSkyCreator>());
    factory.add_creator(std::make_unique<NoEnvirLightWidgetCreaotr>());
}

AGZ_EDITOR_END
