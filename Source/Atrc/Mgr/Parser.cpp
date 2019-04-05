#include <Atrc/Mgr/Parser.h>

namespace Atrc::Mgr::Parser
{

Vec2i ParseVec2i(const ConfigNode &node)
{
    AGZ_HIERARCHY_TRY
    {
        if(auto arr = node.TryAsArray())
        {
            if(arr->Size() == 1)
                return Vec2i((*arr)[0].Parse<int32_t>());
            if(arr->Size() == 2)
                return Vec2i((*arr)[0].Parse<int32_t>(),
                             (*arr)[1].Parse<int32_t>());
        }
        throw std::runtime_error("Invalid vec2i form");
    }
    AGZ_HIERARCHY_WRAP("In parsing vec2i: " + node.ToString())
}

Transform ParseTransform(const ConfigNode &node)
{
    AGZ_HIERARCHY_TRY
    {
        if(!node.IsArray())
            throw std::runtime_error("Array expected");
        auto &arr = node.AsArray();

        Transform ret;
        for(size_t i = 0; i < arr.Size(); ++i)
        {
            if(!arr[i].IsArray())
                throw std::runtime_error("Array expected");
            auto &unit = arr[i].AsArray();

            AGZ_HIERARCHY_TRY
            {
                if(unit.GetTag() == "Translate")
                {
                    if(unit.Size() != 3)
                        throw std::runtime_error("Translate size must be 3");
                    ret = ret * Transform::Translate(Vec3(
                        unit[0].Parse<Real>(),
                        unit[1].Parse<Real>(),
                        unit[2].Parse<Real>()));
                }
                else if(unit.GetTag() == "Rotate")
                {
                    if(unit.Size() != 2)
                        throw std::runtime_error("Rotate size must be 2");
                    ret = ret * Transform::Rotate(
                        ParseVec3(unit[0]), ParseAngle(unit[1]));
                }
                else if(unit.GetTag() == "RotateX")
                {
                    if(unit.Size() != 1)
                        throw std::runtime_error("RotateX size must be 1");
                    ret = ret * Transform::RotateX(ParseAngle(unit[0]));
                }
                else if(unit.GetTag() == "RotateY")
                {
                    if(unit.Size() != 1)
                        throw std::runtime_error("RotateY size must be 1");
                    ret = ret * Transform::RotateY(ParseAngle(unit[0]));
                }
                else if(unit.GetTag() == "RotateZ")
                {
                    if(unit.Size() != 1)
                        throw std::runtime_error("RotateZ size must be 1");
                    ret = ret * Transform::RotateZ(ParseAngle(unit[0]));
                }
                else if(unit.GetTag() == "Scale")
                {
                    if(unit.Size() != 1)
                        throw std::runtime_error("Scale size must be 1");
                    ret = ret * Transform::Scale(unit[0].Parse<Real>());
                }
                else
                    throw std::runtime_error("Unknown transform unit type");
            }
            AGZ_HIERARCHY_WRAP("In parsing transform unit: " + unit.ToString());
        }

        return ret;
    }
    AGZ_HIERARCHY_WRAP("In parsing transform: " + node.ToString())
}

bool ParseBool(const ConfigNode &node)
{
    AGZ_HIERARCHY_TRY
    {
        auto &s = node.AsValue();
        if(s == "True")
            return true;
        if(s == "False")
            return false;
        throw std::runtime_error("Unknown boolean value");
    }
    AGZ_HIERARCHY_WRAP("In parsing bool: " + node.ToString())
}

} // namespace Atrc::Mgr::Parser
