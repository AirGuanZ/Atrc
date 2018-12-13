#include <Atrc/Mgr/Parser.h>

namespace Atrc::Mgr::Parser
{

Spectrum ParseSpectrum(const ConfigNode &node)
{
    ATRC_MGR_TRY
    {
        if(auto arr = node.TryAsArray())
        {
            if(arr->GetTag() == "b")
            {
                if(arr->Size() == 1)
                    return Spectrum((*arr)[0].AsValue().Parse<Real>() / 255);
                if(arr->Size() == 3)
                    return Spectrum((*arr)[0].AsValue().Parse<Real>() / 255,
                                    (*arr)[1].AsValue().Parse<Real>() / 255,
                                    (*arr)[2].AsValue().Parse<Real>() / 255);
            }
            else if(arr->GetTag().Empty())
            {
                if(arr->Size() == 1)
                    return Spectrum((*arr)[0].AsValue().Parse<Real>());
                if(arr->Size() == 3)
                    return Spectrum((*arr)[0].AsValue().Parse<Real>(),
                                    (*arr)[1].AsValue().Parse<Real>(),
                                    (*arr)[2].AsValue().Parse<Real>());
            }
        }

        throw MgrErr("Invalid spectrum form");
    }
    ATRC_MGR_CATCH_AND_RETHROW("In parsing spectrum: " + node.ToString())
}

Vec2i ParseVec2i(const ConfigNode &node)
{
    ATRC_MGR_TRY
    {
        if(auto arr = node.TryAsArray())
        {
            if(arr->Size() == 1)
                return Vec2i((*arr)[0].AsValue().Parse<int32_t>());
            if(arr->Size() == 2)
                return Vec2i((*arr)[0].AsValue().Parse<int32_t>(),
                             (*arr)[1].AsValue().Parse<int32_t>());
        }
        throw MgrErr("Invalid vec2i form");
    }
    ATRC_MGR_CATCH_AND_RETHROW("In parsing vec2i: " + node.ToString())
}

Vec2 ParseVec2(const ConfigNode &node)
{
    ATRC_MGR_TRY
    {
        if(auto arr = node.TryAsArray())
        {
            if(arr->Size() == 1)
                return Vec2((*arr)[0].AsValue().Parse<Real>());
            if(arr->Size() == 2)
                return Vec2((*arr)[0].AsValue().Parse<Real>(),
                            (*arr)[1].AsValue().Parse<Real>());
        }
        throw MgrErr("Invalid vec2 form");
    }
    ATRC_MGR_CATCH_AND_RETHROW("In parsing vec2: " + node.ToString())
}

Vec3 ParseVec3(const ConfigNode &node)
{
    ATRC_MGR_TRY
    {
        if(auto arr = node.TryAsArray())
        {
            if(arr->Size() == 1)
                return Vec3((*arr)[0].AsValue().Parse<Real>());
            if(arr->Size() == 3)
                return Vec3((*arr)[0].AsValue().Parse<Real>(),
                            (*arr)[1].AsValue().Parse<Real>(),
                            (*arr)[2].AsValue().Parse<Real>());
        }
        throw MgrErr("Invalid vec3 form");
    }
    ATRC_MGR_CATCH_AND_RETHROW("In parsing vec3: " + node.ToString())
}

Rad ParseAngle(const ConfigNode &node)
{
    ATRC_MGR_TRY
    {
        if(auto arr = node.TryAsArray())
        {
            if(arr->Size() != 1)
                throw MgrErr("Array is too long");
            if(arr->GetTag() == "Deg")
                return Deg((*arr)[0].AsValue().Parse<Real>());
            else if(arr->GetTag() == "Rad")
                return Rad((*arr)[0].AsValue().Parse<Real>());
        }
        throw MgrErr("Invalid angle form");
    }
    ATRC_MGR_CATCH_AND_RETHROW("In parsing angle: " + node.ToString())
}

Transform ParseTransform(const ConfigNode &node)
{
    ATRC_MGR_TRY
    {
        if(!node.IsArray())
            throw MgrErr("Array expected");
        auto &arr = node.AsArray();

        Transform ret;
        for(size_t i = 0; i < arr.Size(); ++i)
        {
            if(!arr[i].IsArray())
                throw MgrErr("Array expected");
            auto &unit = arr[i].AsArray();

            ATRC_MGR_TRY
            {
                if(unit.GetTag() == "Translate")
                {
                    if(unit.Size() != 3)
                        throw MgrErr("Translate size must be 3");
                    ret = ret * Transform::Translate(Vec3(
                        unit[0].AsValue().Parse<Real>(),
                        unit[1].AsValue().Parse<Real>(),
                        unit[2].AsValue().Parse<Real>()));
                }
                else if(unit.GetTag() == "Rotate")
                {
                    if(unit.Size() != 2)
                        throw MgrErr("Rotate size must be 2");
                    ret = ret * Transform::Rotate(
                        ParseVec3(unit[0]), ParseAngle(unit[1]));
                }
                else if(unit.GetTag() == "RotateX")
                {
                    if(unit.Size() != 1)
                        throw MgrErr("RotateX size must be 1");
                    ret = ret * Transform::RotateX(ParseAngle(unit[0]));
                }
                else if(unit.GetTag() == "RotateY")
                {
                    if(unit.Size() != 1)
                        throw MgrErr("RotateY size must be 1");
                    ret = ret * Transform::RotateY(ParseAngle(unit[0]));
                }
                else if(unit.GetTag() == "RotateZ")
                {
                    if(unit.Size() != 1)
                        throw MgrErr("RotateZ size must be 1");
                    ret = ret * Transform::RotateZ(ParseAngle(unit[0]));
                }
                else if(unit.GetTag() == "Scale")
                {
                    if(unit.Size() != 1)
                        throw MgrErr("Scale size must be 1");
                    ret = ret * Transform::Scale(unit[0].AsValue().Parse<Real>());
                }
                else
                    throw MgrErr("Unknown transform unit type");
            }
            ATRC_MGR_CATCH_AND_RETHROW("In parsing transform unit: " + unit.ToString());
        }

        return ret;
    }
    ATRC_MGR_CATCH_AND_RETHROW("In parsing transform: " + node.ToString())
}

} // namespace Atrc::Mgr::Parser
