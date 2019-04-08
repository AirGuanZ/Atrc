#include <Atrc/Editor/Material/IdealDiffuse.h>

IdealDiffuse::IdealDiffuse(const HasName *creator)
    : IMaterial(creator)
{
    albedo_.SetResource(RF.Get<ITexture>()["Constant"].Create());
}

std::string IdealDiffuse::Save(const std::filesystem::path &relPath) const
{
    static const AGZ::Fmt fmt(
        "type = {};"
        "albedo = {};"
    );
    return Wrap(fmt.Arg(
        GetType(),
        albedo_.GetNoneNullResource()->Save(relPath)));
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
    static const AGZ::Fmt fmt(
        "type = {};"
        "albedo = {};"
    );
    return Wrap(fmt.Arg(
        GetType(),
        albedo_.GetNoneNullResource()->Export(relPath)));
}

void IdealDiffuse::Display()
{
    ImGui::Text("albedo: ");
    if(!albedo_.IsMultiline())
        ImGui::SameLine();
    albedo_.Display();
}

bool IdealDiffuse::IsMultiline() const noexcept
{
    if(auto albedo = albedo_.GetResource())
        return albedo->IsMultiline();
    return false;
}
