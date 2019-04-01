#include <Atrc/Editor/Texture/Constant.h>
#include <Atrc/Editor/Texture/Image.h>
#include <Atrc/Editor/Texture/Range.h>

void RegisterBuiltinTextureCreators(TextureFactory &factory)
{
    static const ConstantCreator iConstantCreator;
    static const ImageCreator iImageCreator;
    static const RangeCreator iRangeCreator;
    factory.AddCreator(&iConstantCreator);
    factory.AddCreator(&iImageCreator);
    factory.AddCreator(&iRangeCreator);
}
