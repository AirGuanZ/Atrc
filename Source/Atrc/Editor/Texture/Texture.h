#pragma once

#include <memory>

#include <AGZUtils/Utils/Config.h>
#include <Atrc/Editor/ResourceInstance/ResourceInstance.h>

class ITexture : public IResource, public ExportAsConfigGroup
{
public:

    using IResource::IResource;

    virtual std::string Save(const std::filesystem::path &relPath) const = 0;

    virtual void Load(const AGZ::ConfigGroup &params, const std::filesystem::path &relPath) = 0;

    virtual std::string Export(const std::filesystem::path &relPath) const = 0;

    virtual void Display() = 0;

    virtual bool IsMultiline() const noexcept = 0;

    virtual void SetRange(float low, float high) { }
};

class ITextureCreator : public IResourceCreator
{
public:

    using Resource = ITexture;

    using IResourceCreator::IResourceCreator;

    virtual std::shared_ptr<ITexture> Create() const = 0;
};

template<typename TTexture>
class DefaultTextureCreator : public ITextureCreator
{
    static_assert(std::is_base_of_v<ITexture, TTexture>);

public:

    using ITextureCreator::ITextureCreator;

    std::shared_ptr<ITexture> Create() const override
    {
        return std::make_shared<TTexture>(this);
    }
};

#define DEFINE_DEFAULT_TEXTURE_CREATOR(TEXTURE, NAME) \
    class TEXTURE##Creator : public DefaultTextureCreator<TEXTURE> \
    { \
    public: \
        TEXTURE##Creator() : DefaultTextureCreator(NAME) { } \
    }

void RegisterBuiltinTextureCreators(TextureFactory &factory);
