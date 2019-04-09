#pragma once

#include <Atrc/Editor/ResourceInstance/ResourceSlot.h>
#include <Atrc/Editor/Material/Material.h>

namespace Atrc::Editor
{

class IdealDiffuse : public ResourceCommonImpl<IMaterial, IdealDiffuse>
{
    ResourceSlot<TextureFactory> albedo_;

public:

    explicit IdealDiffuse(const HasName *creator);

    std::string Save(const std::filesystem::path &relPath) const override;

    void Load(const AGZ::ConfigGroup &params, const std::filesystem::path &relPath) override;

    std::string Export(const std::filesystem::path &relPath) const override;

    void Display() override;

    bool IsMultiline() const noexcept override;
};

DEFINE_DEFAULT_RESOURCE_CREATOR(IMaterialCreator, IdealDiffuse, "IdealDiffuse");

}; // namespace Atrc::Editor
