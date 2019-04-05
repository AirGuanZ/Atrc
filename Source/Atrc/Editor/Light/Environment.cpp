#include <Atrc/Editor/Light/Environment.h>

std::string Environment::Save(const std::filesystem::path &relPath) const
{
    static const AGZ::Fmt fmt(
        "type = {};"
        "tex = {};"
    );
    return Wrap(fmt.Arg(GetType(), tex_.GetNoneNullResource()->Save(relPath)));
}

void Environment::Load(const AGZ::ConfigGroup &params, const std::filesystem::path &relPath)
{
    AGZ_HIERARCHY_TRY

    auto tex = RF.Get<ITexture>()[params["type"].AsValue()].Create();
    tex->Load(params, relPath);
    tex_.SetResource(tex);

    AGZ_HIERARCHY_WRAP("in loading environment light")
}

std::string Environment::Export(const std::filesystem::path &path) const
{
    return Save(path);
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
