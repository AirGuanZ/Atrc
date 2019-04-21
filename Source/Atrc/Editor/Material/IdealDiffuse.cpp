#include <Atrc/Editor/Material/IdealDiffuse.h>
#include <Atrc/Editor/Texture/Texture.h>

namespace Atrc::Editor
{

IdealDiffuse::IdealDiffuse(const HasName *creator)
    : ResourceCommonImpl(creator)
{
    albedo_.SetResource(RF.Get<ITexture>()["Constant"].Create());
}

std::string IdealDiffuse::Save(const std::filesystem::path &relPath) const
{
    AGZ_HIERARCHY_TRY

    static const AGZ::Fmt fmt(
        "type = {};"
        "albedo = {};"
    );
    return Wrap(fmt.Arg(
        GetType(),
        albedo_.GetNoneNullResource()->Save(relPath)));

    AGZ_HIERARCHY_WRAP("in saving ideal diffuse material")
}

void IdealDiffuse::Load(const AGZ::ConfigGroup &params, const std::filesystem::path &relPath)
{
    AGZ_HIERARCHY_TRY

    auto albedo = RF.Get<ITexture>()[params["albedo.type"].AsValue()].Create();
    albedo->Load(params["albedo"].AsGroup(), relPath);
    albedo_.SetResource(albedo);

    AGZ_HIERARCHY_WRAP("in loading ideal diffuse material object with " + params.ToString())
}

std::string IdealDiffuse::Export(const std::filesystem::path &relPath) const
{
    AGZ_HIERARCHY_TRY

    static const AGZ::Fmt fmt(
        "type = {};"
        "albedo = {};"
    );
    return Wrap(fmt.Arg(
        GetType(),
        albedo_.GetNoneNullResource()->Export(relPath)));

    AGZ_HIERARCHY_WRAP("in exporting ideal diffuse material")
}

void IdealDiffuse::Display()
{
    ImGui::Text("albedo: ");
    if(albedo_.IsMultiline())
    {
        ImGui::Indent();
        albedo_.Display();
        ImGui::Unindent();
    }
    else
    {
        ImGui::SameLine();
        albedo_.Display();
    }
}

bool IdealDiffuse::IsMultiline() const noexcept
{
    return albedo_.IsMultiline();
}

}; // namespace Atrc::Editor
