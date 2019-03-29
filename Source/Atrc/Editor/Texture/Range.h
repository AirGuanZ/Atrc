#pragma once

#include <Atrc/Editor/Texture/Texture.h>

class Range : public ITexture
{
    float value_ = 0;
    float low_ = 0, high_ = 1;

public:

    using ITexture::ITexture;

    std::string Save() const override;

    void Load(const AGZ::ConfigGroup &params) override;

    std::string Export() const override;

    void Display() override;

    bool IsMultiline() const noexcept override;

    void SetRange(float low, float high) override;
};

class RangeCreator : public ITextureCreator
{
public:

    RangeCreator() : ITextureCreator("Range") { }

    std::shared_ptr<ITexture> Create(std::string name) const override;
};
