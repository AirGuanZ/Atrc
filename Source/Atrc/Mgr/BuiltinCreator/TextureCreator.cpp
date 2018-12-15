#include <Atrc/Lib/Texture/ConstantTexture.h>
#include <Atrc/Lib/Texture/ImageTexture.h>
#include <Atrc/Mgr/BuiltinCreator/TextureCreator.h>
#include <Atrc/Mgr/Parser.h>

namespace Atrc::Mgr
{

void RegisterBuiltinTextureCreators(Context &context)
{
    static const ConstantTextureCreator constantTextureCreator;
    static const ConstantTexture1Creator constantTextureCreator1;
    context.AddCreator(&constantTextureCreator);
    context.AddCreator(&constantTextureCreator1);
    context.AddCreator(context.CreateWithInteriorArena<ImageTextureCreator>());
}

Texture *ConstantTextureCreator::Create(const ConfigGroup &group, [[maybe_unused]] Context &context, Arena &arena) const
{
    ATRC_MGR_TRY
    {
        auto texel = Parser::ParseSpectrum(group["texel"]);
        return arena.Create<ConstantTexture>(texel);
    }
    ATRC_MGR_CATCH_AND_RETHROW("In creating constant texture: " + group.ToString())
}

Texture *ConstantTexture1Creator::Create(const ConfigGroup &group, [[maybe_unused]] Context &context, Arena &arena) const
{
    ATRC_MGR_TRY
    {
        Real texel = group["texel"].Parse<Real>();
        return arena.Create<ConstantTexture1>(texel);
    }
    ATRC_MGR_CATCH_AND_RETHROW("In creating constant texture: " + group.ToString())
}

Texture *ImageTextureCreator::Create(const ConfigGroup &group, [[maybe_unused]] Context &context, Arena &arena) const
{
    ATRC_MGR_TRY
    {
        const AGZ::Texture2D<Color3b> *tex;

        const Str8 &filename = group["filename"].AsValue();
        auto it = path2Tex_.find(filename);
        if(it != path2Tex_.end())
            tex = it->second;
        else
        {
            auto ttex = arena.Create<AGZ::Texture2D<Color3b>>();
            *ttex = AGZ::Texture2D<Color3b>(AGZ::TextureFile::LoadRGBFromFile(filename));
            tex = ttex;
        }

        auto samplingStrategy = ImageTexture::Linear;
        auto wrappingStrategy = ImageTexture::Clamp;

        if(auto samplerNode = group.Find("sampler"))
        {
            const Str8 &sampler = samplerNode->AsValue();
            if(sampler == "Nearest")
                samplingStrategy = ImageTexture::Nearest;
            else if(sampler == "Linear")
                samplingStrategy = ImageTexture::Linear;
            else
                throw MgrErr("Invalid sampler value");
        }

        if(auto wrapperNode = group.Find("wrapper"))
        {
            const Str8 &wrapper = wrapperNode->AsValue();
            if(wrapper == "Clamp")
                wrappingStrategy = ImageTexture::Clamp;
            else
                throw MgrErr("Invalid wrapper value");
        }

        return arena.Create<ImageTexture>(*tex, samplingStrategy, wrappingStrategy);
    }
    ATRC_MGR_CATCH_AND_RETHROW("In creating image texture: " + group.ToString())
}

} // namespace Atrc::Mgr
