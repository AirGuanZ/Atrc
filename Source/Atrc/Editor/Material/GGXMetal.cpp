#include <Atrc/Editor/Material/GGXMetal.h>

namespace Atrc::Editor
{
    
std::string GGXMetal::Save(const std::filesystem::path &relPath) const
{
    AGZ_HIERARCHY_TRY

    static const AGZ::Fmt fmt(
        "type = {};"
        "rc = {};"
        "roughness = {};"
        "fresnel = {};"
    );
    return Wrap(fmt.Arg(GetType(),
        rc_.GetNoneNullResource()->Save(relPath),
        roughness_.GetNoneNullResource()->Save(relPath),
        fresnel_.GetNoneNullResource()->Save()));

    AGZ_HIERARCHY_WRAP("in saving ggx metal material")
}

void GGXMetal::Load(const AGZ::ConfigGroup &params, const std::filesystem::path &relPath)
{
    AGZ_HIERARCHY_TRY

    auto rc = RF.CreateAndLoad<ITexture>(params["rc"], relPath);
    auto roughness = RF.CreateAndLoad<ITexture>(params["roughness"], relPath);
    auto fresnel = RF.CreateAndLoad<IFresnel>(params["fresnel"]);

    rc_.SetResource(rc);
    roughness_.SetResource(roughness);
    fresnel_.SetResource(fresnel);

    AGZ_HIERARCHY_WRAP("in loading ggx metal material")
}

std::string GGXMetal::Export(const std::filesystem::path &relPath) const
{
    AGZ_HIERARCHY_TRY

    static const AGZ::Fmt fmt(
        "type = GGXMetal;"
        "rc = {};"
        "roughness = {};"
        "fresnel = {};"
    );
    return Wrap(fmt.Arg(
        rc_.GetNoneNullResource()->Export(relPath),
        roughness_.GetNoneNullResource()->Export(relPath),
        fresnel_.GetNoneNullResource()->Export()));

    AGZ_HIERARCHY_WRAP("in exporting ggx metal material")
}

void GGXMetal::Display()
{
    rc_.DisplayAsSubresource("rc");
    roughness_.DisplayAsSubresource("roughness");
    fresnel_.DisplayAsSubresource("fresnel");
}

bool GGXMetal::IsMultiline() const noexcept
{
    return true;
}

} // namespace Atrc::Editor
