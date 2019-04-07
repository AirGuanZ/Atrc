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

DEFINE_DEFAULT_RESOURCE_CREATOR_INTERFACE(ITexture);

void RegisterBuiltinTextureCreators(TextureFactory &factory);
