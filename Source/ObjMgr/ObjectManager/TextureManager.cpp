#include "TextureManager.h"

AGZ_NS_BEG(ObjMgr)

const AGZ::Texture2D<Atrc::Spectrum> *TextureManager::Load(const Str8 &filename)
{
    auto it = path2Tex_.find(filename);
    if(it != path2Tex_.end())
        return it->second;

    try
    {
        auto tex = arena_.Create<AGZ::Texture2D<Atrc::Spectrum>>();
        *tex = AGZ::Texture2D<Atrc::Spectrum>(
            AGZ::TextureFile::LoadRGBFromFile(filename).Map(
                [](const auto &c) { return c.Map([](uint8_t b) { return b / 255.0f; }); }));
        path2Tex_[filename] = tex;
        return tex;
    }
    catch(...)
    {
        return nullptr;
    }
}

AGZ_NS_END(ObjMgr)
