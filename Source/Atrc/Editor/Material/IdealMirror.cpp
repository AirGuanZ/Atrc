#include <AGZUtils/Utils/Exception.h>
#include <Atrc/Editor/Material/IdealMirror.h>

namespace Atrc::Editor
{

IdealMirror::IdealMirror(const HasName *creator)
    : ResourceCommonImpl(creator)
{
    rc_.SetResource(RF.Get<ITexture>()["Constant"].Create());
    fresnel_.SetResource(RF.Get<IFresnel>()["Schlick"].Create());
}

std::string IdealMirror::Save(const std::filesystem::path &relPath) const
{
    AGZ_HIERARCHY_TRY

    static const AGZ::Fmt fmt(
        "type = {};"
        "fresnel = {};"
        "rc = {};"
    );
    return Wrap(fmt.Arg(
        GetType(),
        fresnel_.GetNoneNullResource()->Save(),
        rc_.GetNoneNullResource()->Save(relPath)));

    AGZ_HIERARCHY_WRAP("in saving ideal mirror material")
}

void IdealMirror::Load(const AGZ::ConfigGroup &params, const std::filesystem::path &relPath)
{
    AGZ_HIERARCHY_TRY

    auto fresnel = RF.CreateAndLoad<IFresnel>(params["fresnel"]);
    auto rc      = RF.CreateAndLoad<ITexture>(params["rc"], relPath);

    fresnel_.SetResource(fresnel);
    rc_.SetResource(rc);

    AGZ_HIERARCHY_WRAP("in loading ideal mirror material object with " + params.ToString())
}

std::string IdealMirror::Export(const std::filesystem::path &relPath) const
{
    AGZ_HIERARCHY_TRY

    static const AGZ::Fmt fmt(
        "type = {};"
        "fresnel = {};"
        "rc = {};"
    );
    return Wrap(fmt.Arg(
        GetType(),
        fresnel_.GetNoneNullResource()->Export(),
        rc_.GetNoneNullResource()->Export(relPath)));

    AGZ_HIERARCHY_WRAP("in exporting ideal mirror material")
}

void IdealMirror::Display()
{
    ImGui::Text("rc: ");
    if(rc_.IsMultiline())
    {
        ImGui::Indent();
        rc_.Display();
        ImGui::Unindent();
    }
    else
    {
        ImGui::SameLine();
        rc_.Display();
    }

    ImGui::Text("fresnel: ");
    if(fresnel_.IsMultiline())
    {
        ImGui::Indent();
        fresnel_.Display();
        ImGui::Unindent();
    }
    else
    {
        ImGui::SameLine();
        fresnel_.Display();
    }
}

bool IdealMirror::IsMultiline() const noexcept
{
    return true;
}

}; // namespace Atrc::Editor
