#include <Atrc/Core/Utility/ConfigConvert.h>

namespace Atrc
{
    
std::string Vec2ToConfigStr(const Vec2 &vec)
{
    static const AGZ::TFormatter<char> fmt("({}, {})");
    return fmt.Arg(vec.x, vec.y);
}

Vec2 Node2Vec2(const AGZ::ConfigNode &node)
{
    AGZ_HIERARCHY_TRY

    auto &arr = node.AsArray();
    if(arr.Size() == 1)
        return Vec2(AGZ::From<Real>(arr[0].AsValue()));
    if(arr.Size() == 2)
        return Vec2(
            arr[0].Parse<Real>(),
            arr[1].Parse<Real>());
    throw ConfigConvertException("invalid array size: " + std::to_string(arr.Size()));

    AGZ_HIERARCHY_WRAP("in converting AGZ::ConfigNode to Vec2 with " + node.ToString())
}

} // namespace Atrc
