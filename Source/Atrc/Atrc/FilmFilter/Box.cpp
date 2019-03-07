#include <AGZUtils/Utils/Exception.h>
#include <Atrc/Atrc/FilmFilter/Box.h>
#include <Atrc/Lib/Core/Common.h>

namespace
{
    constexpr double BOX_SIDELEN_MAX = 99;
    constexpr double BOX_SIDELEN_MIN = 0.01;
}

const char *BoxCore::GetTypeName()
{
    return "Box";
}

BoxCore::BoxCore()
{
    ui_.setupUi(this);
    ui_.sidelen->setMaximum(BOX_SIDELEN_MAX);
    ui_.sidelen->setMinimum(BOX_SIDELEN_MIN);
    ui_.sidelen->setSingleStep(0.05);
}

std::string BoxCore::Serialize() const
{
    static const AGZ::TFormatter<char> fmt(
        "type = Box;"
        "sidelen = {};");
    return "{" + fmt.Arg(ui_.sidelen->value()) + "}";
}

void BoxCore::Deserialize(const AGZ::ConfigNode &node)
{
    AGZ_HIERARCHY_TRY

    auto &param = node.AsGroup();
    AGZ_ASSERT(param["type"].AsValue() == "Box");

    auto sidelen = param["sidelen"].Parse<double>();
    if(sidelen < BOX_SIDELEN_MIN || sidelen > BOX_SIDELEN_MAX)
        throw AGZ::HierarchyException("invalid sidelen value: " + std::to_string(sidelen));
    ui_.sidelen->setValue(sidelen);

    AGZ_HIERARCHY_WRAP("in deserializing box filmfilter: " + node.ToString())
}
