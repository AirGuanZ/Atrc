#include <Atrc/Editor/Light/Light.h>

#include <Atrc/Editor/Light/Environment.h>

void RegisterBuiltinLightCreators(LightFactory &factory)
{
    static const EnvironmentCreator iEnvironmentCreator;
    factory.AddCreator(&iEnvironmentCreator);
}
