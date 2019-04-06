#pragma once

#include <memory>

#include <AGZUtils/Utils/Config.h>
#include <Atrc/Editor/ResourceInstance/ResourceInstance.h>

class ISampler : public IResource, public ExportAsConfigGroup
{
public:

    using IResource::IResource;

    virtual std::string Save() const = 0;

    virtual void Load(const AGZ::ConfigGroup &params) = 0;

    virtual std::string Export() const = 0;

    virtual void Display() = 0;

    virtual bool IsMultiline() const noexcept = 0;
};

class ISamplerCreator : public IResourceCreator
{
public:

    using Resource = ISampler;

    using IResourceCreator::IResourceCreator;

    virtual std::shared_ptr<ISampler> Create() const = 0;
};

template<typename TSampler>
class DefaultSamplerCreator : public ISamplerCreator
{
    static_assert(std::is_base_of_v<ISampler, TSampler>);

public:

    using ISamplerCreator::ISamplerCreator;

    std::shared_ptr<ISampler> Create() const override
    {
        return std::make_shared<TSampler>(this);
    }
};

#define DEFINE_DEFAULT_SAMPLER_CREATOR(SAMPLER, NAME) \
    class SAMPLER##Creator : public DefaultSamplerCreator<SAMPLER> \
    { \
    public: \
        SAMPLER##Creator() : DefaultSamplerCreator(NAME) { } \
    }

void RegisterBuiltinSamplerCreators(SamplerFactory &factory);
