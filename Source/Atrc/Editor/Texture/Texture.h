#pragma once

#include <memory>

#include <AGZUtils/Utils/Config.h>
#include <Atrc/Editor/ResourceInstance/ResourceInstance.h>

class ITexture : public IResource, public ExportAsConfigGroup
{
public:

    using IResource::IResource;

    virtual std::string Save() const = 0;

    virtual void Load(const AGZ::ConfigGroup &params) = 0;

    virtual std::string Export() const = 0;

    virtual void Display() = 0;

    virtual bool IsMultiline() const noexcept = 0;

    virtual void SetRange(float low, float high) { }
};

class ITextureCreator : public IResourceCreator
{
public:

    using Resource = ITexture;

    using IResourceCreator::IResourceCreator;

    virtual std::shared_ptr<ITexture> Create(std::string name) const = 0;
};

void RegisterBuiltinTextureCreators(TextureFactory &factory);
