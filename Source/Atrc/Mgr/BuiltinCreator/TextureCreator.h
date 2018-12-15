#pragma once

#include <Utils/Texture.h>

#include <Atrc/Mgr/Context.h>

namespace Atrc::Mgr
{

void RegisterBuiltinTextureCreators(Context &context);

/*
    type = Constant

    texel = Spectrum
*/
class ConstantTextureCreator : public Creator<Texture>
{
public:

    Str8 GetTypeName() const override { return "Constant"; }

    Texture *Create(const ConfigGroup &group, Context &context, Arena &arena) const override;
};

/*
    type = Constant1

    texel = Real
*/
class ConstantTexture1Creator : public Creator<Texture>
{
public:

    Str8 GetTypeName() const override { return "Constant1"; }

    Texture *Create(const ConfigGroup &group, Context &context, Arena &arena) const override;
};

/*
    type = Image

    filename = String
    sampler  = Nearest | Linear
    wrapper  = Clamp
*/
class ImageTextureCreator : public Creator<Texture>
{
    mutable std::unordered_map<Str8, const AGZ::Texture2D<Color3b>*> path2Tex_;

public:

    Str8 GetTypeName() const override { return "Image"; }

    Texture *Create(const ConfigGroup &group, Context &context, Arena &arena) const override;
};

} // namespace Atrc::Mgr
