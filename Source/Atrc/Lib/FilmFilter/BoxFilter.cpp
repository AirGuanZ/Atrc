#include <AGZUtils/String/StdStr.h>
#include <Atrc/Lib/FilmFilter/BoxFilter.h>
#include <Atrc/Lib/Utility/ConfigConvert.h>

namespace Atrc
{

std::string BoxFilterData::Serialize() const
{
    static const AGZ::TFormatter<char> fmt(
        "type = Box;"
        "radius = {};");
    return "{" + fmt.Arg(Vec2ToConfigStr(radius_)) + "}";
}

void BoxFilterData::Deserialize(const AGZ::ConfigGroup &param)
{
    AGZ_ASSERT(param["type"] == GetTypeName());
    radius_ = Node2Vec2(param["radius"]);
}

BoxFilter::BoxFilter(const Vec2 &radius) noexcept
    : FilmFilter(radius)
{

}

Real BoxFilter::Eval([[maybe_unused]] Real relX, [[maybe_unused]] Real relY) const noexcept
{
    return 1;
}

} // namespace Atrc
