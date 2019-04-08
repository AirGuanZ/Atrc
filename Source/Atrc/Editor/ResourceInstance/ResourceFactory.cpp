#include <Atrc/Editor/ResourceInstance/ResourceFactory.h>

namespace Atrc::Editor
{

ResourceFactoryList RF;

void RegisterBuiltinResourceCreators()
{
    RegisterBuiltinFilmFilterCreators(RF.Get<IFilmFilter>());
    RegisterBuiltinFresnelCreators   (RF.Get<IFresnel>());
    RegisterBuiltinLightCreators     (RF.Get<ILight>());
    RegisterBuiltinMaterialCreators  (RF.Get<IMaterial>());
    RegisterBuiltinSamplerCreators   (RF.Get<ISampler>());
    RegisterBuiltinTextureCreators   (RF.Get<ITexture>());
}

}; // namespace Atrc::Editor
