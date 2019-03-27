#include <Atrc/Editor/FilmFilter/Box.h>
#include <Atrc/Editor/GL.h>

std::string Box::ExportImpl() const
{
    static const AGZ::Fmt fmt(
        "type = {};"
        "radius = {};"
    );
    return fmt.Arg(GetType(), radius_);
}

std::string Box::Save() const
{
    return Export();
}

void Box::Load(const AGZ::ConfigGroup &params)
{
    AGZ_HIERARCHY_TRY

    radius_ = params["radius"].Parse<float>();
    if(radius_ <= 0)
        throw AGZ::HierarchyException("invalid radius value: " + std::to_string(radius_));

    AGZ_HIERARCHY_WRAP("in loading box film filter with " + params.ToString())
}

void Box::Display()
{
    ImGui::PushID(this);
    AGZ_SCOPE_GUARD({ ImGui::PopID(); });

    ImGui::PushItemWidth(-1);
    AGZ_SCOPE_GUARD({ ImGui::PopItemWidth(); });
    ImGui::InputFloat("radius", &radius_);
}

bool Box::IsMultiline() const noexcept
{
    return false;
}
