#include <Atrc/Editor/Sampler/Sampler.h>

#include <Atrc/Editor/Sampler/Native.h>

void RegisterBuiltinSamplerCreators(SamplerFactory &factory)
{
    static const NativeCreator iNativeCreator;
    factory.AddCreator(&iNativeCreator);
}
