#include <Atrc/Editor/Texture/Range.h>
#include <Atrc/Editor/GL.h>

std::string Range::Save() const
{
    static const AGZ::Fmt fmt(
        "type = {};"
        "low = {};"
        "high = {};"
        "value = {};");
    return Wrap(fmt.Arg(GetType(), low_, high_, value_));
}

void Range::Load(const AGZ::ConfigGroup &params)
{
    AGZ_HIERARCHY_TRY

    low_ = params["low"].Parse<float>();
    high_ = params["high"].Parse<float>();
    value_ = params["value"].Parse<float>();

    if(low_ > high_)
        throw AGZ::HierarchyException("invalid range parameter (low > high)");
    if(value_ < low_ || value_ > high_)
        throw AGZ::HierarchyException("invalid range parameter (value \notin [low, high])");

    AGZ_HIERARCHY_WRAP("in loading range texture with " + params.ToString())
}

std::string Range::Export() const
{
    static const AGZ::Fmt fmt(
        "type = Constant1;"
        "texel = {};");
    return Wrap(fmt.Arg(value_));
}

void Range::Display()
{
    ImGui::Text("value"); ImGui::SameLine();

    ImGui::PushID(0);
    ImGui::PushItemWidth(-1);
    AGZ_SCOPE_GUARD({ ImGui::PopItemWidth(); ImGui::PopID(); });

    ImGui::SliderFloat("", &value_, low_, high_);
}

bool Range::IsMultiline() const noexcept
{
    return false;
}

void Range::SetRange(float low, float high)
{
    AGZ_ASSERT(low <= high);
    low_ = low;
    high_ = high;
    value_ = AGZ::Math::Clamp(value_, low, high);
}

std::shared_ptr<ITexture> RangeCreator::Create(std::string name) const
{
    return std::make_shared<Range>(std::move(name), this);
}
