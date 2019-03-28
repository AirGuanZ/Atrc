#include <Atrc/Editor/FilmFilter/Box.h>
#include <Atrc/Editor/GL.h>

std::string Box::ExportImpl() const
{
    static const AGZ::Fmt fmt(
        "type = {};"
        "sidelen = {};"
    );
    return fmt.Arg(GetType(), sidelen_);
}

std::string Box::Save() const
{
    return Export();
}

void Box::Load(const AGZ::ConfigGroup &params)
{
    AGZ_HIERARCHY_TRY

    sidelen_ = params["sidelen"].Parse<float>();
    if(sidelen_ <= 0)
        throw AGZ::HierarchyException("invalid sidelen value: " + std::to_string(sidelen_));

    AGZ_HIERARCHY_WRAP("in loading box film filter with " + params.ToString())
}

void Box::Display()
{
    ImGui::Text("sidelen"); ImGui::SameLine();

    ImGui::PushID(0);
    ImGui::PushItemWidth(-1);
    AGZ_SCOPE_GUARD({ ImGui::PopItemWidth(); ImGui::PopID(); });

    ImGui::InputFloat("", &sidelen_);
}

bool Box::IsMultiline() const noexcept
{
    return false;
}

std::shared_ptr<IFilmFilter> BoxCreator::Create() const
{
    return std::make_shared<Box>("", this);
}
