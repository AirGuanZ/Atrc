#pragma once

#include <Atrc/Editor/Texture/Texture.h>
#include <Atrc/Editor/GL.h>

namespace Atrc::Editor
{

class Constant1 : public ResourceCommonImpl<ITexture, Constant1>
{
    float texel_ = 0;
    float low_ = 1, high_ = 0;

public:

    using ResourceCommonImpl<ITexture, Constant1>::ResourceCommonImpl;

    std::string Save(const std::filesystem::path &relPath) const override;

    void Load(const AGZ::ConfigGroup &params, const std::filesystem::path &relPath) override;

    std::string Export(const std::filesystem::path &relPath) const override;

    void Display() override;

    bool IsMultiline() const noexcept override;

    void SetRange(float low, float high) override;
};

DEFINE_DEFAULT_RESOURCE_CREATOR(ITextureCreator, Constant1, "Constant1");

}; // namespace Atrc::Editor
