#include <Atrc/Editor/ResourceInstance/ResourceFactory.h>
#include <Atrc/Editor/Texture/Constant.h>
#include <Atrc/Editor/Texture/Constant1.h>
#include <Atrc/Editor/Texture/HDR.h>
#include <Atrc/Editor/Texture/Image.h>

namespace Atrc::Editor
{

void RegisterBuiltinTextureCreators(TextureFactory &factory)
{
    static const ConstantCreator iConstantCreator;
    static const Constant1Creator iConstant1Creator;
    static const HDRCreator iHDRCreator;
    static const ImageCreator iImageCreator;
    factory.AddCreator(&iConstantCreator);
    factory.AddCreator(&iConstant1Creator);
    factory.AddCreator(&iHDRCreator);
    factory.AddCreator(&iImageCreator);
}

}; // namespace Atrc::Editor
