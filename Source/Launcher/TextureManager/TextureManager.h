#pragma once

#include <unordered_map>

#include "../Common.h"

class TextureManager : public AGZ::Singleton<TextureManager>
{
	ObjArena<> arena_;
	std::unordered_map<Str8, const AGZ::Texture2D<Atrc::Spectrum>*> path2Tex_;

public:

	bool Load(const Str8 &filename);

	const AGZ::Texture2D<Atrc::Spectrum> *operator[](const Str8 &path) const;
};
