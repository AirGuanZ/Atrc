#pragma once

#include <unordered_map>

#include "../Common.h"

AGZ_NS_BEG(ObjMgr)

using TextureCreator = ObjectCreator<Atrc::Texture>;
using TextureManager = ObjectManager<Atrc::Texture>;

// value = Spectrum
class ConstantTextureCreator : public TextureCreator, public AGZ::Singleton<ConstantTextureCreator>
{
public:

    Str8 GetName() const override { return "Constant"; }

    Atrc::Texture *Create(const ConfigGroup &params, ObjArena<> &arena) const override;
};

// filename = "path"
// sampler = Nearest/Linear
// wrapper = Clamp
class ImageTextureCreator : public TextureCreator, public AGZ::Singleton<ImageTextureCreator>
{
public:

    Str8 GetName() const override { return "Image"; }

    Atrc::Texture *Create(const ConfigGroup &params, ObjArena<> &arena) const override;
};

AGZ_NS_END(ObjMgr)
