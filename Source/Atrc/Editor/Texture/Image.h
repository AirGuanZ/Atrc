#pragma once

#include <Atrc/Editor/Texture/Texture.h>
#include <Atrc/Editor/FileSelector.h>
#include <Atrc/Editor/GL.h>

class Image : public ITexture
{
public:

    using ITexture::ITexture;

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
    };

    std::shared_ptr<GLTextureWithFilename> glTex_;
    FileSelector fileSelector_;

    bool SetGLTextureFilename(const std::filesystem::path &filename);
};

DEFINE_DEFAULT_TEXTURE_CREATOR(Image, "Image");
