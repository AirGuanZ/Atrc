#pragma once

#include <unordered_map>

#include "../Common.h"

class TextureManager : public AGZ::Singleton<TextureManager>
{
	ObjArena<> arena_;
	std::unordered_map<Str8, const AGZ::Texture2D<Atrc::Spectrum>*> path2Tex_;

public:

    const AGZ::Texture2D<Atrc::Spectrum> *Load(const Str8 &filename);
};
