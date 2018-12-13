#pragma once

#include <Atrc/Mgr/Context.h>

namespace Atrc::Mgr
{

/*
    type = Constant

    texel = Spectrum
*/
class ConstantTextureCreator : public Creator<Texture>
{
public:

    Str8 GetTypeName() const override { return "Constant"; }

    const Texture *Create(const ConfigGroup &group, Context &context, Arena &arena) override;
};

/*
    type = Image

    filename = String
    sampler  = Nearest | Linear
    wrapper  = Clamp
*/
class ImageTextureCreator : public Creator<Texture>
{
    std::unordered_map<Str8, const AGZ::Texture2D<Color3b>*> path2Tex_;

public:

    Str8 GetTypeName() const override { return "Image"; }

    const Texture *Create(const ConfigGroup &group, Context &context, Arena &arena) override;
};

} // namespace Atrc::Mgr
