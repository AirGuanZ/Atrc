#include <AGZUtils/Utils/Exception.h>
#include <Atrc/Editor/Sampler/Native.h>
#include <Atrc/Editor/GL.h>

namespace Atrc::Editor
{

std::string Native::Save() const
{
    static const AGZ::Fmt fmt(
        "type = {};"
        "spp = {};"
    );
    auto s = fmt.Arg(GetType(), spp_);
    if(withSeed_)
        s += "seed=" + std::to_string(seed_);
    return Wrap(s);
}

void Native::Load(const AGZ::ConfigGroup &params)
{
    AGZ_HIERARCHY_TRY

    spp_ = params["spp"].Parse<int>();
    if(auto seedNode = params.Find("seed"))
    {
        withSeed_ = true;
        seed_ = seedNode->Parse<int>();
    }
    else
    {
        withSeed_ = false;
        seed_ = 42;
    }

    AGZ_HIERARCHY_WRAP("in loading native sampler with " + params.ToString())
}

std::string Native::Export() const
{
    return Save();
}

void Native::Display()
{
    ImGui::InputInt("spp", &spp_, 0);
    ImGui::Checkbox("with seed", &withSeed_);
    if(withSeed_)
    {
        ImGui::SameLine();
        ImGui::InputInt("seed", &seed_, 0);
    }
}

bool Native::IsMultiline() const noexcept
{
    return true;
}

}; // namespace Atrc::Editor
