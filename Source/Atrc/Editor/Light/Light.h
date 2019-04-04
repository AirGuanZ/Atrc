#pragma once

#include <memory>

#include <AGZUtils/Utils/Config.h>
#include <Atrc/Editor/ResourceInstance/ResourceInstance.h>

class ILight : public IResource, public ExportAsConfigGroup
{
public:

    using IResource::IResource;

    virtual std::string Save(const std::filesystem::path &relPath) const = 0;

    virtual void Load(const AGZ::ConfigGroup &params, const std::filesystem::path &relPath) = 0;

    virtual std::string Export(const std::filesystem::path &path) const = 0;

    virtual void Display() = 0;

    virtual bool IsMultiline() const noexcept = 0;
};

class ILightCreator : public IResourceCreator
{
public:

    using Resource = ILight;

    using IResourceCreator::IResourceCreator;

    virtual std::shared_ptr<ILight> Create() const = 0;
};

template<typename TLight>
class DefaultLightCreator : public ILightCreator
{
public:

    using ILightCreator::ILightCreator;

    std::shared_ptr<ILight> Create() const override
    {
        return std::make_shared<TLight>(std::move(name), this);
    }
};

#define DEFINE_DEFAULT_LIGHT_CREATOR(LIGHT, NAME) \
    class LIGHT##Creator : public DefaultLightCreator<LIGHT> \
    { \
    public: \
        LIGHT##Creator() : DefaultLightCreator(NAME) { } \
    }

void RegisterBuiltinLightCreators(LightFactory &factory);
