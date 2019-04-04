#include <Atrc/Editor/ResourceInstance/ResourceFactory.h>

ResourceFactoryList RF;

void RegisterBuiltinResourceCreators()
{
    RegisterBuiltinFilmFilterCreators(RF.Get<IFilmFilter>());
    RegisterBuiltinLightCreators     (RF.Get<ILight>());
    RegisterBuiltinMaterialCreators  (RF.Get<IMaterial>());
    RegisterBuiltinTextureCreators   (RF.Get<ITexture>());
}
