#include <Atrc/Editor/Texture/Constant.h>
#include <Atrc/Editor/Texture/Range.h>

void RegisterBuiltinTextureCreators(TextureFactory &factory)
{
    static const ConstantCreator iConstantCreator;
    static const RangeCreator iRangeCreator;
    factory.AddCreator(&iConstantCreator);
    factory.AddCreator(&iRangeCreator);
}
