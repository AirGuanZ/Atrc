#include <Atrc/Core/Utility/ConfigConvert.h>
#include <Atrc/Editor/Texture/Constant.h>

namespace Atrc::Editor
{

std::string Constant::Save(const std::filesystem::path &relPath) const
{
    AGZ_HIERARCHY_TRY

    static const AGZ::Fmt fmt(
        "type = {};"
        "asColor = {};"
        "texel = {};"
    );
    return Wrap(fmt.Arg(
        GetType(),
        BoolToCS(asColor_),
        Vec3fToCS(texel_)));

    AGZ_HIERARCHY_WRAP("in saving constant texture")
}

void Constant::Load(const AGZ::ConfigGroup &params, const std::filesystem::path &relPath)
{
    AGZ_HIERARCHY_TRY

    asColor_ = Atrc::Node2Bool(params["asColor"]);
    texel_ = Atrc::Node2Vec3f(params["texel"]);

    AGZ_HIERARCHY_WRAP("in loading constant texture with " + params.ToString())
}

std::string Constant::Export(const std::filesystem::path &relPath) const
{
    AGZ_HIERARCHY_TRY

    static const AGZ::Fmt fmt(
        "type = Constant;"
        "texel = {};"
    );
    return Wrap(fmt.Arg(Vec3fToCS(texel_)));

    AGZ_HIERARCHY_WRAP("in exporting constant texture")
}

void Constant::Display()
{
    ImGui::PushID(0);
    ImGui::Checkbox("", &asColor_); ImGui::SameLine();
    ImGui::PopID();

    ImGui::PushID(1);
    ImGui::PushItemWidth(-1);
    if(asColor_)
        ImGui::ColorEdit3("", &texel_[0], ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
    else
        ImGui::InputFloat3("", &texel_[0]);
    ImGui::PopItemWidth();
    ImGui::PopID();
}

bool Constant::IsMultiline() const noexcept
{
    return false;
}

}; // namespace Atrc::Editor
