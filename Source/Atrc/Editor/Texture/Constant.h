#pragma once

#include <Atrc/Editor/Texture/Texture.h>
#include <Atrc/Editor/GL.h>

class Constant : public ITexture
{
    bool asColor_ = true;
    Vec3f texel_ = Vec3f();

public:

    using ITexture::ITexture;

    std::string Save() const override;

    void Load(const AGZ::ConfigGroup &params) override;

    std::string Export() const override;

    void Display() override;

    bool IsMultiline() const noexcept override;
};

class ConstantCreator : public ITextureCreator
{
public:

    ConstantCreator() : ITextureCreator("Constant") { }

    std::shared_ptr<ITexture> Create(std::string name) const override;
};
