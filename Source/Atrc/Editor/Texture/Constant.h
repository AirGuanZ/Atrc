#pragma once

#include <Atrc/Editor/Texture/Texture.h>
#include <Atrc/Editor/GL.h>

namespace Atrc::Editor
{

class Constant : public ITexture
{
    bool asColor_ = true;
    Vec3f texel_ = Vec3f();

public:

    using ITexture::ITexture;

    std::string Save(const std::filesystem::path &relPath) const override;

    void Load(const AGZ::ConfigGroup &params, const std::filesystem::path &relPath) override;

    std::string Export(const std::filesystem::path &relPath) const override;

    void Display() override;

    bool IsMultiline() const noexcept override;
};

DEFINE_DEFAULT_RESOURCE_CREATOR(ITextureCreator, Constant, "Constant");

}; // namespace Atrc::Editor
