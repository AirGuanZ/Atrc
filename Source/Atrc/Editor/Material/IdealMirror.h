#pragma once

#include <Atrc/Editor/ResourceInstance/ResourceSlot.h>
#include <Atrc/Editor/Fresnel/Fresnel.h>
#include <Atrc/Editor/Material/Material.h>
#include <Atrc/Editor/Texture/Texture.h>

namespace Atrc::Editor
{

class IdealMirror : public ResourceCommonImpl<IMaterial, IdealMirror>
{
    FresnelSlot fresnel_;
    TextureSlot rc_;

public:

    IdealMirror(const HasName *creator);

    std::string Save(const std::filesystem::path &relPath) const override;

    void Load(const AGZ::ConfigGroup &params, const std::filesystem::path &relPath) override;

    std::string Export(const std::filesystem::path &relPath) const override;

    void Display() override;

    bool IsMultiline() const noexcept override;
};

DEFINE_DEFAULT_RESOURCE_CREATOR(IMaterialCreator, IdealMirror, "IdealMirror");

}; // namespace Atrc::Editor
