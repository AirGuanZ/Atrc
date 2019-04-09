#pragma once

#include <Atrc/Editor/ResourceInstance/ResourceSlot.h>
#include <Atrc/Editor/Material/Material.h>

namespace Atrc::Editor
{
    
class DisneyReflection : public ResourceCommonImpl<IMaterial, DisneyReflection>
{
    TextureSlot baseColor_;
    TextureSlot metallic_;
    TextureSlot subsurface_;
    TextureSlot specular_;
    TextureSlot specularTint_;
    TextureSlot roughness_;
    TextureSlot anisotropic_;
    TextureSlot sheen_;
    TextureSlot sheenTint_;
    TextureSlot clearcoat_;
    TextureSlot clearcoatGloss_;

    static void LimitRange(ITexture &tex);

    static void DisplayTexture(const char *attribName, TextureSlot &slot);

public:

    explicit DisneyReflection(const HasName *creator);

    std::string Save(const std::filesystem::path &relPath) const override;

    void Load(const AGZ::ConfigGroup &params, const std::filesystem::path &relPath) override;

    std::string Export(const std::filesystem::path &relPath) const override;

    void Display() override;

    bool IsMultiline() const noexcept override;
};

DEFINE_DEFAULT_RESOURCE_CREATOR(IMaterialCreator, DisneyReflection, "DisneyReflection");

} // namespace Atrc::Editor
