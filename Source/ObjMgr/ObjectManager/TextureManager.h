#pragma once

#include <unordered_map>

#include "../Common.h"

AGZ_NS_BEG(ObjMgr)

class TextureManager : public AGZ::Singleton<TextureManager>
{
    ObjArena<> arena_;
    std::unordered_map<Str8, const AGZ::Texture2D<Atrc::Spectrum>*> path2Tex_;

public:

    const AGZ::Texture2D<Atrc::Spectrum> *Load(const Str8 &filename);
};

AGZ_NS_END(ObjMgr)
