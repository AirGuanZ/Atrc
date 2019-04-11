#include <AGZUtils/Utils/Exception.h>
#include <Atrc/Editor/Fresnel/Dielectric.h>
#include <Atrc/Editor/GL.h>

namespace Atrc::Editor
{

std::string Dielectric::Save() const
{
    static const AGZ::Fmt fmt(
        "type = {};"
        "etaI = {};"
        "etaT = {};"
    );
    return Wrap(fmt.Arg(GetType(), etaOut_, etaIn_));
}

void Dielectric::Load(const AGZ::ConfigGroup &params)
{
    AGZ_HIERARCHY_TRY

    etaOut_ = params["etaI"].Parse<float>();
    etaIn_ = params["etaT"].Parse<float>();

    AGZ_HIERARCHY_WRAP("in loading fresnel dielectric obejct with " + params.ToString())
}

std::string Dielectric::Export() const
{
    return Save();
}

void Dielectric::Display()
{
    ImGui::InputFloat("outside eta", &etaOut_);
    ImGui::InputFloat("inside  eta", &etaIn_);
}

bool Dielectric::IsMultiline() const noexcept
{
    return true;
}

}; // namespace Atrc::Editor
