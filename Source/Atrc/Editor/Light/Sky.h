#pragma once

#include <Atrc/Editor/Light/Light.h>
#include <Atrc/Editor/ResourceInstance/ResourceSlot.h>

namespace Atrc::Editor
{

class Sky : public ResourceCommonImpl<ILight, Sky>
{
    Vec3f top_;
    Vec3f bottom_;

public:

    using ResourceCommonImpl<ILight, Sky>::ResourceCommonImpl;

    std::string Save(const std::filesystem::path &relPath) const override;

    void Load(const AGZ::ConfigGroup &params, const std::filesystem::path &relPath) override;

    std::string Export(const std::filesystem::path &path) const override;

    void Display() override;

    bool IsMultiline() const noexcept override;
};

DEFINE_DEFAULT_RESOURCE_CREATOR(ILightCreator, Sky, "Sky");

}; // namespace Atrc::Editor
