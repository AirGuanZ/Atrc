#pragma once

#include <Atrc/Editor/Light/Light.h>
#include <Atrc/Editor/GL.h>
#include <Lib/ImFileBrowser/imfilebrowser.h>

namespace Atrc::Editor
{
    
class SHEnv : public ResourceCommonImpl<ILight, SHEnv>
{
    int SHOrder_ = 1;
    std::vector<Vec3f> coefs_ = { Vec3f() };

    ImGui::FileBrowser fileBrowser_;

public:

    using ResourceCommonImpl<ILight, SHEnv>::ResourceCommonImpl;

    std::string Save(const std::filesystem::path &relPath) const override;

    void Load(const AGZ::ConfigGroup &params, const std::filesystem::path &relPath) override;

    std::string Export(const std::filesystem::path &path) const override;

    void Display() override;

    bool IsMultiline() const noexcept override;
};

DEFINE_DEFAULT_RESOURCE_CREATOR(ILightCreator, SHEnv, "SHEnv");

} // namespace Atrc::Editor
