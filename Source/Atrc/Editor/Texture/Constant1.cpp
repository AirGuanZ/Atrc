#include <AGZUtils/Utils/Exception.h>
#include <Atrc/Editor/Texture/Constant1.h>

namespace Atrc::Editor
{

std::string Constant1::Save(const std::filesystem::path &relPath) const
{
    static const AGZ::Fmt fmt(
        "type = {};"
        "low = {};"
        "high = {};"
        "texel = {};");
    return Wrap(fmt.Arg(GetType(), low_, high_, texel_));
}

void Constant1::Load(const AGZ::ConfigGroup &params, const std::filesystem::path &relPath)
{
    AGZ_HIERARCHY_TRY

    low_ = params["low"].Parse<float>();
    high_ = params["high"].Parse<float>();
    texel_ = params["texel"].Parse<float>();

    if(low_ <= high_)
    {
        if(texel_ < low_ || texel_ > high_)
            throw std::runtime_error("invalid range parameter (value \notin [low, high])");
    }

    AGZ_HIERARCHY_WRAP("in loading constant1 texture with " + params.ToString())
}

std::string Constant1::Export(const std::filesystem::path &relPath) const
{
    static const AGZ::Fmt fmt(
        "type = Constant1;"
        "low = {};"
        "high = {};"
        "texel = {};");
    return Wrap(fmt.Arg(low_, high_, texel_));
}

void Constant1::Display()
{
    ImGui::PushID(0);
    ImGui::PushItemWidth(-1);
    AGZ_SCOPE_GUARD({ ImGui::PopItemWidth(); ImGui::PopID(); });

    if(low_ < high_)
        ImGui::SliderFloat("", &texel_, low_, high_);
    else
        ImGui::InputFloat("", &texel_);
}

bool Constant1::IsMultiline() const noexcept
{
    return false;
}

void Constant1::SetRange(float low, float high)
{
    low_ = low;
    high_ = high;
    if(low_ < high_)
        texel_ = AGZ::Math::Clamp(texel_, low_, high_);
}

}; // namespace Atrc::Editor
