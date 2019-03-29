#include <Atrc/Editor/FilmFilter/Gaussian.h>
#include <Atrc/Editor/GL.h>

std::string Gaussian::Save() const
{
    return Export();
}

void Gaussian::Load(const AGZ::ConfigGroup &params)
{
    AGZ_HIERARCHY_TRY

    radius_ = params["radius"].Parse<float>();
    if(radius_ <= 0)
        throw AGZ::HierarchyException("invalid radius value: " + std::to_string(radius_));

    alpha_ = params["alpha"].Parse<float>();
    if(alpha_ <= 0)
        throw AGZ::HierarchyException("invalid alpha value: " + std::to_string(alpha_));

    AGZ_HIERARCHY_WRAP("in loading box film filter with " + params.ToString())
}

std::string Gaussian::Export() const
{
    static const AGZ::Fmt fmt(
        "type = {};"
        "radius = {};"
        "alpha = {};"
    );
    return Wrap(fmt.Arg(GetType(), radius_, alpha_));
}

void Gaussian::Display()
{
    {
        ImGui::Text("radius"); ImGui::SameLine();

        ImGui::PushID(0); ImGui::PushItemWidth(-1);
        AGZ_SCOPE_GUARD({ ImGui::PopItemWidth(); ImGui::PopID(); });

        ImGui::InputFloat("", &radius_);
    }

    {
        ImGui::Text("alpha "); ImGui::SameLine();

        ImGui::PushID(1); ImGui::PushItemWidth(-1);
        AGZ_SCOPE_GUARD({ ImGui::PopItemWidth(); ImGui::PopID(); });

        ImGui::InputFloat("", &alpha_);
    }
}

bool Gaussian::IsMultiline() const noexcept
{
    return true;
}

std::shared_ptr<IFilmFilter> GaussianCreator::Create() const
{
    return std::make_shared<Gaussian>("", this);
}
