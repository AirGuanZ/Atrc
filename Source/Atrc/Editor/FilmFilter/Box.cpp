#include <AGZUtils/Utils/Exception.h>
#include <Atrc/Editor/FilmFilter/Box.h>
#include <Atrc/Editor/GL.h>

namespace Atrc::Editor
{

std::string Box::Save() const
{
    return Export();
}

void Box::Load(const AGZ::ConfigGroup &params)
{
    AGZ_HIERARCHY_TRY

    sidelen_ = params["sidelen"].Parse<float>();
    if(sidelen_ <= 0)
        throw std::runtime_error("invalid sidelen value: " + std::to_string(sidelen_));

    AGZ_HIERARCHY_WRAP("in loading box film filter with " + params.ToString())
}

std::string Box::Export() const
{
    static const AGZ::Fmt fmt(
        "type = {};"
        "sidelen = {};"
    );
    return Wrap(fmt.Arg(GetType(), sidelen_));
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

}; // namespace Atrc::Editor
