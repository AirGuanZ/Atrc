#pragma once

#include <Atrc/Editor/Light/Light.h>
#include <Atrc/Editor/ResourceInstance/ResourceSlot.h>

namespace Atrc::Editor
{

class Environment : public ResourceCommonImpl<ILight, Environment>
{
    ResourceSlot<TextureFactory> tex_;

public:

    using ResourceCommonImpl<ILight, Environment>::ResourceCommonImpl;

    std::string Save(const std::filesystem::path &relPath) const override;

    void Load(const AGZ::ConfigGroup &params, const std::filesystem::path &relPath) override;

    std::string Export(const std::filesystem::path &path) const override;

    void Display() override;

    bool IsMultiline() const noexcept override;
};

DEFINE_DEFAULT_RESOURCE_CREATOR(ILightCreator, Environment, "Environment");

}; // namespace Atrc::Editor
