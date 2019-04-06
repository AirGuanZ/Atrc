#pragma once

#include <Atrc/Editor/Sampler/Sampler.h>

class Native : public ISampler
{
    bool withSeed_ = false;
    int seed_ = 42;

    int spp_ = 100;

public:

    using ISampler::ISampler;

    std::string Save() const override;

    void Load(const AGZ::ConfigGroup &params) override;

    std::string Export() const override;

    void Display() override;

    bool IsMultiline() const noexcept override;
};

DEFINE_DEFAULT_SAMPLER_CREATOR(Native, "Native");
