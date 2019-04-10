#include <AGZUtils/Utils/Exception.h>
#include <Atrc/Editor/Material/DisneyReflection.h>

namespace Atrc::Editor
{

void DisneyReflection::LimitRange(ITexture &tex)
{
    tex.SetRange(0, 1);
}

void DisneyReflection::DisplayTexture(const char *attribName, TextureSlot &slot)
{
    ImGui::Text(attribName);
    if(slot.IsMultiline())
    {
        ImGui::Indent();
        slot.Display();
        ImGui::Unindent();
    }
    else
    {
        ImGui::SameLine();
        slot.Display();
    }
}

DisneyReflection::DisneyReflection(const HasName *creator)
    : ResourceCommonImpl(creator)
{
    baseColor_     .SetResource(RF.Create<ITexture>("Constant"));
    metallic_      .SetResource(RF.Create<ITexture>("Constant1"));
    subsurface_    .SetResource(RF.Create<ITexture>("Constant1"));
    specular_      .SetResource(RF.Create<ITexture>("Constant1"));
    specularTint_  .SetResource(RF.Create<ITexture>("Constant1"));
    roughness_     .SetResource(RF.Create<ITexture>("Constant1"));
    anisotropic_   .SetResource(RF.Create<ITexture>("Constant1"));
    sheen_         .SetResource(RF.Create<ITexture>("Constant1"));
    sheenTint_     .SetResource(RF.Create<ITexture>("Constant1"));
    clearcoat_     .SetResource(RF.Create<ITexture>("Constant1"));
    clearcoatGloss_.SetResource(RF.Create<ITexture>("Constant1"));

    metallic_      .SetResourceChangedCallback(&DisneyReflection::LimitRange);
    subsurface_    .SetResourceChangedCallback(&DisneyReflection::LimitRange);
    specular_      .SetResourceChangedCallback(&DisneyReflection::LimitRange);
    specularTint_  .SetResourceChangedCallback(&DisneyReflection::LimitRange);
    roughness_     .SetResourceChangedCallback(&DisneyReflection::LimitRange);
    anisotropic_   .SetResourceChangedCallback(&DisneyReflection::LimitRange);
    sheen_         .SetResourceChangedCallback(&DisneyReflection::LimitRange);
    sheenTint_     .SetResourceChangedCallback(&DisneyReflection::LimitRange);
    clearcoat_     .SetResourceChangedCallback(&DisneyReflection::LimitRange);
    clearcoatGloss_.SetResourceChangedCallback(&DisneyReflection::LimitRange);
}

std::string DisneyReflection::Save(const std::filesystem::path &relPath) const
{
    AGZ_HIERARCHY_TRY

    static const AGZ::Fmt fmt(
        "type = {};"
        "baseColor = {};"
        "metallic = {};"
        "subsurface = {};"
        "specular = {};"
        "specularTint = {};"
        "roughness = {};"
        "anisotropic = {};"
        "sheen = {};"
        "sheenTint = {};"
        "clearcoat = {};"
        "clearcoatGloss = {};"
    );
    return Wrap(fmt.Arg(
        GetType(),
        baseColor_     .GetNoneNullResource()->Save(relPath),
        metallic_      .GetNoneNullResource()->Save(relPath),
        subsurface_    .GetNoneNullResource()->Save(relPath),
        specular_      .GetNoneNullResource()->Save(relPath),
        specularTint_  .GetNoneNullResource()->Save(relPath),
        roughness_     .GetNoneNullResource()->Save(relPath),
        anisotropic_   .GetNoneNullResource()->Save(relPath),
        sheen_         .GetNoneNullResource()->Save(relPath),
        sheenTint_     .GetNoneNullResource()->Save(relPath),
        clearcoat_     .GetNoneNullResource()->Save(relPath),
        clearcoatGloss_.GetNoneNullResource()->Save(relPath)
    ));

    AGZ_HIERARCHY_WRAP("in saving disney reflection object material")
}

void DisneyReflection::Load(const AGZ::ConfigGroup &params, const std::filesystem::path &relPath)
{
    AGZ_HIERARCHY_TRY

    baseColor_     .SetResource(RF.CreateAndLoad<ITexture>(params["baseColor"], relPath));
    metallic_      .SetResource(RF.CreateAndLoad<ITexture>(params["metallic"], relPath));
    subsurface_    .SetResource(RF.CreateAndLoad<ITexture>(params["subsurface"], relPath));
    specular_      .SetResource(RF.CreateAndLoad<ITexture>(params["specular"], relPath));
    specularTint_  .SetResource(RF.CreateAndLoad<ITexture>(params["specularTint"], relPath));
    roughness_     .SetResource(RF.CreateAndLoad<ITexture>(params["roughness"], relPath));
    anisotropic_   .SetResource(RF.CreateAndLoad<ITexture>(params["anisotropic"], relPath));
    sheen_         .SetResource(RF.CreateAndLoad<ITexture>(params["sheen"], relPath));
    sheenTint_     .SetResource(RF.CreateAndLoad<ITexture>(params["sheenTint"], relPath));
    clearcoat_     .SetResource(RF.CreateAndLoad<ITexture>(params["clearcoat"], relPath));
    clearcoatGloss_.SetResource(RF.CreateAndLoad<ITexture>(params["clearcoatGloss"], relPath));
    
    AGZ_HIERARCHY_WRAP("in loading disney reflection object material")
}

std::string DisneyReflection::Export(const std::filesystem::path &relPath) const
{
    AGZ_HIERARCHY_TRY

    static const AGZ::Fmt fmt(
        "type = DisneyReflection;"
        "baseColor = {};"
        "metallic = {};"
        "subsurface = {};"
        "specular = {};"
        "specularTint = {};"
        "roughness = {};"
        "anisotropic = {};"
        "sheen = {};"
        "sheenTint = {};"
        "clearcoat = {};"
        "clearcoatGloss = {};"
    );
    return Wrap(fmt.Arg(
        baseColor_     .GetNoneNullResource()->Export(relPath),
        metallic_      .GetNoneNullResource()->Export(relPath),
        subsurface_    .GetNoneNullResource()->Export(relPath),
        specular_      .GetNoneNullResource()->Export(relPath),
        specularTint_  .GetNoneNullResource()->Export(relPath),
        roughness_     .GetNoneNullResource()->Export(relPath),
        anisotropic_   .GetNoneNullResource()->Export(relPath),
        sheen_         .GetNoneNullResource()->Export(relPath),
        sheenTint_     .GetNoneNullResource()->Export(relPath),
        clearcoat_     .GetNoneNullResource()->Export(relPath),
        clearcoatGloss_.GetNoneNullResource()->Export(relPath)
    ));

    AGZ_HIERARCHY_WRAP("in exporting disney reflection object material")
}

void DisneyReflection::Display()
{
    DisplayTexture("baseColor:      ", baseColor_);
    DisplayTexture("metallic:       ", metallic_);
    DisplayTexture("subsurface:     ", subsurface_);
    DisplayTexture("specular:       ", specular_);
    DisplayTexture("specularTint    ", specularTint_);
    DisplayTexture("roughness:      ", roughness_);
    DisplayTexture("anisotropic:    ", anisotropic_);
    DisplayTexture("sheen:          ", sheen_);
    DisplayTexture("sheenTint:      ", sheenTint_);
    DisplayTexture("clearcoat:      ", clearcoat_);
    DisplayTexture("clearcoatGloss: ", clearcoatGloss_);
}

bool DisneyReflection::IsMultiline() const noexcept
{
    return true;
}

} // namespace Atrc::Editor
