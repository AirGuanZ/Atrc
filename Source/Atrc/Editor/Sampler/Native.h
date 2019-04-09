#pragma once

#include <Atrc/Editor/Sampler/Sampler.h>

namespace Atrc::Editor
{

class Native : public ResourceCommonImpl<ISampler, Native>
{
    bool withSeed_ = false;
    int seed_ = 42;

    int spp_ = 100;

public:

    using ResourceCommonImpl<ISampler, Native>::ResourceCommonImpl;

    std::string Save() const override;

    void Load(const AGZ::ConfigGroup &params) override;

    std::string Export() const override;

    void Display() override;

    bool IsMultiline() const noexcept override;
};

DEFINE_DEFAULT_RESOURCE_CREATOR(ISamplerCreator, Native, "Native");

}; // namespace Atrc::Editor
