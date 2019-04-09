#pragma once

#include <memory>

#include <AGZUtils/Utils/Config.h>
#include <Atrc/Editor/ResourceInstance/ResourceInstance.h>

namespace Atrc::Editor
{

class ISampler : public ResourceCommon<ISampler>
{
public:

    using ResourceCommon<ISampler>::ResourceCommon;

    static const char *StrID() noexcept { return "sampler"; }

    virtual std::string Save() const = 0;

    virtual void Load(const AGZ::ConfigGroup &params) = 0;

    virtual std::string Export() const = 0;

    virtual void Display() = 0;

    virtual bool IsMultiline() const noexcept = 0;
};

DEFINE_DEFAULT_RESOURCE_CREATOR_INTERFACE(ISampler);

void RegisterBuiltinSamplerCreators(SamplerFactory &factory);

}; // namespace Atrc::Editor
