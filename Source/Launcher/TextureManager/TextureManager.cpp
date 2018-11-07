#include "TextureManager.h"

bool TextureManager::Load(const Str8 &filename)
{
	if(path2Tex_.find(filename) != path2Tex_.end())
		return true;

	try
	{
		auto tex = arena_.Create<AGZ::Texture2D<Atrc::Spectrum>>();
		*tex = AGZ::Texture2D<Atrc::Spectrum>(
			AGZ::TextureFile::LoadRGBFromFile(filename.ToStdWString()).Map(
				[](const auto &c) { return c.Map([](uint8_t b) { return b / 255.0f; }); }));
		path2Tex_[filename] = tex;
	}
	catch(...)
	{
		return false;
	}

	return true;
}

const AGZ::Texture2D<Atrc::Spectrum> *TextureManager::operator[](const Str8 &path) const
{
	auto it = path2Tex_.find(path);
	return it != path2Tex_.end() ? it->second : nullptr;
}
