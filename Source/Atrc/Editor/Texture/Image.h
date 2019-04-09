#pragma once

#include <Atrc/Editor/Texture/Texture.h>
#include <Atrc/Editor/FileSelector.h>
#include <Atrc/Editor/GL.h>

namespace Atrc::Editor
{

class Image : public ResourceCommonImpl<ITexture, Image>
{
public:

    using ResourceCommonImpl<ITexture, Image>::ResourceCommonImpl;

    Image(const Image &copyFrom);

    std::string Save(const std::filesystem::path &relPath) const override;

    void Load(const AGZ::ConfigGroup &params, const std::filesystem::path &relPath) override;

    std::string Export(const std::filesystem::path &relPath) const override;

    void Display() override;

    bool IsMultiline() const noexcept override;

private:

    struct GLTextureWithFilename
    {
        std::filesystem::path filename;
        GL::Texture2D tex;
        float WOverH;
    };

    std::shared_ptr<GLTextureWithFilename> glTex_;
    FileSelector fileSelector_;

    bool SetGLTextureFilename(const std::filesystem::path &filename);
};

DEFINE_DEFAULT_RESOURCE_CREATOR(ITextureCreator, Image, "Image");

}; // namespace Atrc::Editor
