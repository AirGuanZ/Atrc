#include "ParamParser.h"

AGZ_NS_BEG(ObjMgr)

using namespace Atrc;

Spectrum ParamParser::ParseSpectrum(const ConfigNode &node)
{
    try
    {
        if(node.IsValue())
            return Spectrum(node.AsValue().Parse<float>());

        if(node.IsArray())
        {
            auto &arr = node.AsArray();

            if(!arr.Size())
                return Spectrum(0.0f);

            if(arr.Size() == 1)
                return Spectrum(arr[0].AsValue().Parse<float>());

            if(arr[0].AsValue() == "b")
            {
                if(arr.Size() == 2)
                    return Spectrum(arr[1].AsValue().Parse<float>() / 255.0f);
                if(arr.Size() == 4)
                    return 1 / 255.0f * Spectrum(arr[1].AsValue().Parse<float>(),
                                                 arr[2].AsValue().Parse<float>(),
                                                 arr[3].AsValue().Parse<float>());
            }
            else
            {
                if(arr.Size() == 3)
                    return Spectrum(arr[0].AsValue().Parse<float>(),
                                    arr[1].AsValue().Parse<float>(),
                                    arr[2].AsValue().Parse<float>());
            }
        }
    }
    catch(const std::exception &err)
    {
        throw SceneInitializationException(err.what());
    }

    throw ParamParsingError("ParamParser: unknown spectrum form");
}

Vec3 ParamParser::ParseVec3(const ConfigNode &node)
{
    try
    {
        auto &arr = node.AsArray();
        if(arr.Size() != 3)
            throw ParamParsingError("ParamParser: unknown vec3 form");

        return Vec3(arr[0].AsValue().Parse<Real>(),
                    arr[1].AsValue().Parse<Real>(),
                    arr[2].AsValue().Parse<Real>());
    }
    catch(const std::exception &err)
    {
        throw SceneInitializationException(err.what());
    }
}

Transform ParamParser::ParseTransform(const ConfigNode &node)
{
    try
    {
        auto &arr = node.AsArray();
        Transform ret;

        auto ExtractAngle = [&](const AGZ::ConfigArray &a) -> Rad
        {
            if(a.Size() == 1)
            {
                if(a.GetTag() == "Deg")
                    return Deg(a[0].AsValue().Parse<Real>());
                if(a.GetTag() == "Rad")
                    return Rad(a[0].AsValue().Parse<Real>());
            }
            throw ParamParsingError("ParamParser: unknown angle form");
        };

        for(size_t i = 0; i < arr.Size(); ++i)
        {

            auto &t = arr[i].AsArray();
            if(!t.Size())
                throw ParamParsingError("ParamParser: unknown transform form");

            if(t.GetTag() == "Scale")
            {
                if(t.Size() != 1)
                    throw ParamParsingError("ParamParser: unknown scaling form");
                ret = ret * Transform::Scale(t[0].AsValue().Parse<Real>());
            }
            else if(t.GetTag() == "Translate")
            {
                if(t.Size() != 3)
                    throw ParamParsingError("ParamParser: unknown translating form");
                ret = ret * Transform::Translate(
                    t[0].AsValue().Parse<Real>(),
                    t[1].AsValue().Parse<Real>(),
                    t[2].AsValue().Parse<Real>());
            }
            else if(t.GetTag() == "RotateX")
            {
                ret = ret * Transform::RotateX(ExtractAngle(t[0].AsArray()));
            }
            else if(t.GetTag() == "RotateY")
            {
                ret = ret * Transform::RotateY(ExtractAngle(t[0].AsArray()));
            }
            else if(t.GetTag() == "RotateZ")
            {
                ret = ret * Transform::RotateZ(ExtractAngle(t[0].AsArray()));
            }
            else if(t.GetTag() == "Rotate")
            {
                if(t.Size() != 2)
                    throw ParamParsingError("ParamParser: unknown rotate form");

                auto axis = ParseVec3(t[0]);
                Rad rad = ExtractAngle(t[1].AsArray());

                ret = ret * Transform::Rotate(axis, rad);
            }
            else
                throw ParamParsingError("ParamParser: unknown transform form");
        }

        return ret;
    }
    catch(const std::exception &err)
    {
        throw SceneInitializationException(err.what());
    }
}

AGZ_NS_END(ObjMgr)
