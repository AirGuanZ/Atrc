#include <Atrc/Editor/Light/Light.h>

#include <Atrc/Editor/Light/Env.h>
#include <Atrc/Editor/Light/SHEnv.h>
#include <Atrc/Editor/Light/Sky.h>

namespace Atrc::Editor
{

void RegisterBuiltinLightCreators(LightFactory &factory)
{
    static const EnvironmentCreator iEnvironmentCreator;
    static const SHEnvCreator iSHEnvCreator;
    static const SkyCreator iSkyCreator;
    factory.AddCreator(&iEnvironmentCreator);
    factory.AddCreator(&iSHEnvCreator);
    factory.AddCreator(&iSkyCreator);
}

}; // namespace Atrc::Editor
