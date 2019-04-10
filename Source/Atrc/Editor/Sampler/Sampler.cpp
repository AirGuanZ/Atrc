#include <Atrc/Editor/ResourceInstance/ResourceFactory.h>
#include <Atrc/Editor/Sampler/Sampler.h>

#include <Atrc/Editor/Sampler/Native.h>

namespace Atrc::Editor
{

void RegisterBuiltinSamplerCreators(SamplerFactory &factory)
{
    static const NativeCreator iNativeCreator;
    factory.AddCreator(&iNativeCreator);
}

}; // namespace Atrc::Editor
