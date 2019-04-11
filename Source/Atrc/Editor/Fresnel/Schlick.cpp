#include <AGZUtils/Utils/Exception.h>
#include <Atrc/Editor/Fresnel/Schlick.h>
#include <Atrc/Editor/GL.h>

namespace Atrc::Editor
{

std::string Schlick::Save() const
{
    static const AGZ::Fmt fmt(
        "type = {};"
        "etaI = {};"
        "etaT = {};"
    );
    return Wrap(fmt.Arg(GetType(), etaOut_, etaIn_));
}

void Schlick::Load(const AGZ::ConfigGroup &params)
{
    AGZ_HIERARCHY_TRY

    etaOut_ = params["etaI"].Parse<float>();
    etaIn_ = params["etaT"].Parse<float>();

    AGZ_HIERARCHY_WRAP("in loading fresnel schlick obejct with " + params.ToString())
}

std::string Schlick::Export() const
{
    return Save();
}

void Schlick::Display()
{
    ImGui::InputFloat("outside eta", &etaOut_);
    ImGui::InputFloat("inside  eta", &etaIn_);
}

bool Schlick::IsMultiline() const noexcept
{
    return true;
}

}; // namespace Atrc::Editor
