#pragma once

#include <Atrc/Editor/Texture/Texture.h>

class Range : public ITexture
{
    float value_ = 0;
    float low_ = 0, high_ = 1;

public:

    using ITexture::ITexture;

    std::string Save(const std::filesystem::path &relPath) const override;

    void Load(const AGZ::ConfigGroup &params, const std::filesystem::path &relPath) override;

    std::string Export(const std::filesystem::path &relPath) const override;

    void Display() override;

    bool IsMultiline() const noexcept override;

    void SetRange(float low, float high) override;
};

DEFINE_DEFAULT_TEXTURE_CREATOR(Range, "Range");
