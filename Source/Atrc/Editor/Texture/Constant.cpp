#include <Atrc/Core/Utility/ConfigConvert.h>
#include <Atrc/Editor/Texture/Constant.h>

std::string Constant::Save() const
{
    static const AGZ::Fmt fmt(
        "type = {};"
        "asColor = {};"
        "texel = {};"
    );
    return Wrap(fmt.Arg(
        GetType(),
        Atrc::BoolToCS(asColor_),
        Atrc::Vec3fToCS(texel_)));
}

void Constant::Load(const AGZ::ConfigGroup &params)
{
    AGZ_HIERARCHY_TRY

    asColor_ = Atrc::Node2Bool(params["asColor"]);
    texel_ = Atrc::Node2Vec3f(params["texel"]);

    AGZ_HIERARCHY_WRAP("in loading constant texture with " + params.ToString())
}

std::string Constant::Export() const
{
    static const AGZ::Fmt fmt(
        "type = Constant;"
        "texel = {};"
    );
    return Wrap(fmt.Arg(Atrc::Vec3fToCS(texel_)));
}

void Constant::Display()
{
    ImGui::PushID(0);
    ImGui::Checkbox("", &asColor_); ImGui::SameLine();
    ImGui::PopID();

    ImGui::PushID(1);
    if(asColor_)
        ImGui::ColorEdit3("", &texel_[0], ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
    else
        ImGui::InputFloat3("", &texel_[0]);
    ImGui::PopID();
}

bool Constant::IsMultiline() const noexcept
{
    return false;
}

std::shared_ptr<ITexture> ConstantCreator::Create(std::string name) const
{
    return std::make_shared<Constant>(std::move(name), this);
}
