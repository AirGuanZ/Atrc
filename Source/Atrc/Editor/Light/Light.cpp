#include <Atrc/Editor/Light/Light.h>

#include <Atrc/Editor/Light/Environment.h>
#include <Atrc/Editor/Light/Sky.h>

void RegisterBuiltinLightCreators(LightFactory &factory)
{
    static const EnvironmentCreator iEnvironmentCreator;
    static const SkyCreator iSkyCreator;
    factory.AddCreator(&iEnvironmentCreator);
    factory.AddCreator(&iSkyCreator);
}
