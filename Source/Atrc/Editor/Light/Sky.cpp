#include <Atrc/Core/Utility/ConfigConvert.h>
#include <Atrc/Editor/Light/Sky.h>

namespace Atrc::Editor
{

std::string Sky::Save(const std::filesystem::path &relPath) const
{
    static const AGZ::Fmt fmt(
        "type = {};"
        "top = {};"
        "bottom = {};"
    );
    return Wrap(fmt.Arg(GetType(), Vec3fToCS(top_), Vec3fToCS(bottom_)));
}

void Sky::Load(const AGZ::ConfigGroup &params, const std::filesystem::path &relPath)
{
    AGZ_HIERARCHY_TRY

    top_ = Node2Vec3f(params["top"]);
    bottom_ = Node2Vec3f(params["bottom"]);

    AGZ_HIERARCHY_WRAP("in loading sky light with " + params.ToString())
}

std::string Sky::Export(const std::filesystem::path &path) const
{
    return Save(path);
}

void Sky::Display()
{
    ImGui::ColorEdit3(" top  ", &top_[0], ImGuiColorEditFlags_HDR);
    ImGui::ColorEdit3("bottom", &bottom_[0], ImGuiColorEditFlags_HDR);
}

bool Sky::IsMultiline() const noexcept
{
    return true;
}

}; // namespace Atrc::Editor
