#pragma once

#include <Atrc/Editor/Light/Light.h>
#include <Atrc/Editor/ResourceInstance/ResourceSlot.h>

namespace Atrc::Editor
{

class CubeEnv : public ResourceCommonImpl<ILight, CubeEnv>
{
    ResourceSlot<TextureFactory> posX_;
    ResourceSlot<TextureFactory> posY_;
    ResourceSlot<TextureFactory> posZ_;
    ResourceSlot<TextureFactory> negX_;
    ResourceSlot<TextureFactory> negY_;
    ResourceSlot<TextureFactory> negZ_;
    float rotateYDeg_ = 0;

public:

    using ResourceCommonImpl<ILight, CubeEnv>::ResourceCommonImpl;

    std::string Save(const std::filesystem::path &relPath) const override;

    void Load(const AGZ::ConfigGroup &params, const std::filesystem::path &relPath) override;

    std::string Export(const std::filesystem::path &path) const override;

    void Display() override;

    bool IsMultiline() const noexcept override;
};

DEFINE_DEFAULT_RESOURCE_CREATOR(ILightCreator, CubeEnv, "CubeEnvironment");

} // namespace Atrc::Editor

