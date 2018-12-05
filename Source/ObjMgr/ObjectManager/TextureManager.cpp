#include "../ParamParser.h"
#include "TextureManager.h"

AGZ_NS_BEG(ObjMgr)

class TextureLoader : public AGZ::Singleton<TextureLoader>
{
    ObjArena<> arena_;

    std::unordered_map<Str8, const AGZ::Texture2D<Atrc::Color3f>*> path2Tex3f_;
    std::unordered_map<Str8, const AGZ::Texture2D<Atrc::Color3b>*> path2Tex3b_;

public:

    const AGZ::Texture2D<Atrc::Color3f> *Load3f(const Str8 &filename);

    const AGZ::Texture2D<Atrc::Color3b> *Load3b(const Str8 &filename);
};

const AGZ::Texture2D<Atrc::Spectrum> *TextureLoader::Load3f(const Str8 &filename)
{
    auto it = path2Tex3f_.find(filename);
    if(it != path2Tex3f_.end())
        return it->second;

    try
    {
        auto tex = arena_.Create<AGZ::Texture2D<Atrc::Spectrum>>();
        *tex = AGZ::Texture2D<Atrc::Spectrum>(
            AGZ::TextureFile::LoadRGBFromFile(filename).Map(
                [](const auto &c) { return c.Map([](uint8_t b) { return b / 255.0f; }); }));
        path2Tex3f_[filename] = tex;
        return tex;
    }
    catch(...)
    {
        return nullptr;
    }
}

const AGZ::Texture2D<Atrc::Color3b> *TextureLoader::Load3b(const Str8 &filename)
{
    auto it = path2Tex3b_.find(filename);
    if(it != path2Tex3b_.end())
        return it->second;

    try
    {
        auto tex = arena_.Create<AGZ::Texture2D<Atrc::Color3b>>();
        *tex = AGZ::Texture2D<Atrc::Color3b>(AGZ::TextureFile::LoadRGBFromFile(filename));
        path2Tex3b_[filename] = tex;
        return tex;
    }
    catch(...)
    {
        return nullptr;
    }
}

Atrc::Texture *ConstantTextureCreator::Create(const ConfigGroup &params, ObjArena<> &arena) const
{
    auto value = ParamParser::ParseSpectrum(params["value"]);
    return arena.Create<Atrc::ConstantTexture>(value);
}

Atrc::Texture *ImageTextureCreator::Create(const ConfigGroup &params, ObjArena<> &arena) const
{
    auto filename = params["filename"].AsValue();
    auto sampler  = params["sampler"].AsValue();
    auto wrapper  = params["wrapper"].AsValue();

    auto tex = TextureLoader::GetInstance().Load3b(filename);
    if(!tex)
        throw SceneInitializationException("Failed to load texture file from: " + filename);
    
    Atrc::ImageTexture::SampleStrategy sampleStrategy;
    if(sampler == "Nearest")
        sampleStrategy = Atrc::ImageTexture::Nearest;
    else if(sampler == "Linear")
        sampleStrategy = Atrc::ImageTexture::Linear;
    else
        throw SceneInitializationException("ImageTextureCreator: unknown sampling strategy");

    Atrc::ImageTexture::WrapStrategy wrapStrategy;
    if(wrapper == "Clamp")
        wrapStrategy = Atrc::ImageTexture::Clamp;
    else
        throw SceneInitializationException("ImageTextureCreator: unknown wrapping strategy");
    
    return arena.Create<Atrc::ImageTexture>(*tex, sampleStrategy, wrapStrategy);
}

AGZ_NS_END(ObjMgr)
