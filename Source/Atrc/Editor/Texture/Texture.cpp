#include <Atrc/Editor/Texture/Range.h>

void RegisterBuiltinTextureCreators(TextureFactory &factory)
{
    static const RangeCreator iRangeCreator;
    factory.AddCreator(&iRangeCreator);
}
