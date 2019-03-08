#include <AGZUtils/Utils/Exception.h>
#include <Atrc/Atrc/FilmFilter/Gaussian.h>
#include <Atrc/Lib/Core/Common.h>

namespace
{
    constexpr double GAUSSIAN_RADIUS_MAX = 99;
    constexpr double GAUSSIAN_RADIUS_MIN = 0.01;

    constexpr double GAUSSIAN_ALPHA_MAX = 10;
    constexpr double GAUSSIAN_ALPHA_MIN = 0.01;
}

const char *GaussianCore::GetTypeName()
{
    return "Gaussian";
}

GaussianCore::GaussianCore()
{
    ui_.setupUi(this);

    ui_.radiusSpinBox->setMaximum(GAUSSIAN_RADIUS_MAX);
    ui_.radiusSpinBox->setMinimum(GAUSSIAN_RADIUS_MIN);
    ui_.radiusSpinBox->setSingleStep(0.05);

    ui_.alphaSpinBox->setMaximum(GAUSSIAN_ALPHA_MAX);
    ui_.alphaSpinBox->setMinimum(GAUSSIAN_ALPHA_MIN);
    ui_.alphaSpinBox->setSingleStep(0.03);
}

std::string GaussianCore::Serialize() const
{
    static const AGZ::TFormatter<char> fmt(
        "type = Gaussian;"
        "radius = {};"
        "alpha = {};");
    return "{" + fmt.Arg(ui_.radiusSpinBox->value(), ui_.alphaSpinBox->value()) + "}";
}

void GaussianCore::Deserialize(const AGZ::ConfigNode &node)
{
    AGZ_HIERARCHY_TRY

        auto &param = node.AsGroup();
    AGZ_ASSERT(param["type"].AsValue() == "Gaussian");

    auto radius = param["radius"].Parse<double>();
    if(radius < GAUSSIAN_RADIUS_MIN || radius > GAUSSIAN_RADIUS_MAX)
        throw AGZ::HierarchyException("invalid radius value: " + std::to_string(radius));
    ui_.radiusSpinBox->setValue(radius);

    auto alpha = param["alpha"].Parse<double>();
    if(alpha < GAUSSIAN_ALPHA_MIN || alpha > GAUSSIAN_ALPHA_MAX)
        throw AGZ::HierarchyException("invalid alpha value: " + std::to_string(alpha));
    ui_.alphaSpinBox->setValue(alpha);

    AGZ_HIERARCHY_WRAP("in deserializing gaussian filmfilter: " + node.ToString())
}

bool GaussianCore::IsMultiline() const noexcept
{
    return true;
}
