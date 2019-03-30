#include <Atrc/Core/Utility/ConfigConvert.h>

namespace Atrc
{
    
std::string Vec2ToCS(const Vec2 &vec)
{
    static const AGZ::TFormatter<char> fmt("({}, {})");
    return fmt.Arg(vec.x, vec.y);
}

std::string Vec3ToCS(const Vec3 &vec)
{
    static const AGZ::TFormatter<char> fmt("({}, {}, {})");
    return fmt.Arg(vec.x, vec.y, vec.z);
}

std::string Vec3fToCS(const AGZ::Math::Vec3f &vec)
{
    static const AGZ::TFormatter<char> fmt("({}, {}, {})");
    return fmt.Arg(vec.x, vec.y, vec.z);
}

std::string BoolToCS(bool value)
{
    return value ? "True" : "False";
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

AGZ::Math::Vec3f Node2Vec3f(const AGZ::ConfigNode &node)
{
    AGZ_HIERARCHY_TRY

    auto &arr = node.AsArray();
    if(arr.Size() == 1)
        return AGZ::Math::Vec3f(AGZ::From<Real>(arr[0].AsValue()));
    if(arr.Size() == 3)
        return AGZ::Math::Vec3f(
            arr[0].Parse<Real>(),
            arr[1].Parse<Real>(),
            arr[2].Parse<Real>());
    throw ConfigConvertException("invalid array size: " + std::to_string(arr.Size()));

    AGZ_HIERARCHY_WRAP("in converting AGZ::ConfigNode to Vec3f with " + node.ToString())
}

bool Node2Bool(const AGZ::ConfigNode &node)
{
    AGZ_HIERARCHY_TRY

    auto &str = node.AsValue();
    if(str == "True")
        return true;
    if(str == "False")
        return false;
    throw ConfigConvertException("invalid bool value: " + str);

    AGZ_HIERARCHY_WRAP("in converting AGZ::ConfigNode to bool with " + node.ToString())
}

} // namespace Atrc
