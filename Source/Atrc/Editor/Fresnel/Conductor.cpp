#include <Atrc/Core/Utility/ConfigConvert.h>
#include <Atrc/Editor/Fresnel/Conductor.h>

namespace Atrc::Editor
{

std::string Conductor::Save() const
{
    static const AGZ::Fmt fmt(
        "type = {};"
        "etaI = {};"
        "etaT = {};"
        "k = {};"
    );
    return Wrap(fmt.Arg(
        GetType(),
        Atrc::Vec3fToCS(etaOut_),
        Atrc::Vec3fToCS(etaIn_),
        Atrc::Vec3fToCS(k_)));
}

void Conductor::Load(const AGZ::ConfigGroup &params)
{
    AGZ_HIERARCHY_TRY

    etaOut_ = Atrc::Node2Vec3f(params["etaI"]);
    etaIn_  = Atrc::Node2Vec3f(params["etaT"]);
    k_      = Atrc::Node2Vec3f(params["k"]);

    AGZ_HIERARCHY_WRAP("in loading conductor fresnel object")
}

std::string Conductor::Export() const
{
    return Save();
}

void Conductor::Display()
{
    ImGui::InputFloat3("outside eta", &etaOut_[0]);
    ImGui::InputFloat3("inside  eta", &etaIn_[0]);
    ImGui::InputFloat3("k          ", &k_[0]);
}

bool Conductor::IsMultiline() const noexcept
{
    return true;
}

}; // namespace Atrc::Editor
