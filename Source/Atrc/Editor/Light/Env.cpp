#include <Atrc/Editor/Light/Env.h>

namespace Atrc::Editor
{

std::string Environment::Save(const std::filesystem::path &relPath) const
{
    AGZ_HIERARCHY_TRY

    static const AGZ::Fmt fmt(
        "type = {};"
        "tex = {};"
    );
    return Wrap(fmt.Arg(GetType(), tex_.GetNoneNullResource()->Save(relPath)));

    AGZ_HIERARCHY_WRAP("in saving environment light")
}

void Environment::Load(const AGZ::ConfigGroup &params, const std::filesystem::path &relPath)
{
    AGZ_HIERARCHY_TRY

    auto tex = RF.Get<ITexture>()[params["tex.type"].AsValue()].Create();
    tex->Load(params["tex"].AsGroup(), relPath);
    tex_.SetResource(tex);

    AGZ_HIERARCHY_WRAP("in loading environment light with " + params.ToString())
}

std::string Environment::Export(const std::filesystem::path &path) const
{
    AGZ_HIERARCHY_TRY

    static const AGZ::Fmt fmt(
        "type = {};"
        "tex = {};"
    );
    return Wrap(fmt.Arg(GetType(), tex_.GetNoneNullResource()->Export(path)));

    AGZ_HIERARCHY_WRAP("in exporting environment light")
}

void Environment::Display()
{
    ImGui::Text("tex: "); ImGui::SameLine();
    tex_.Display();
}

bool Environment::IsMultiline() const noexcept
{
    if(tex_.GetResource())
        return tex_.GetResource()->IsMultiline();
    return false;
}

}; // namespace Atrc::Editor
