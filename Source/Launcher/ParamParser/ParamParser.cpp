#include "ParamParser.h"

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

        for(size_t i = 0; i < arr.Size(); ++i)
        {
            auto ExtractAngle = [&](const AGZ::ConfigArray &a) -> Rad
            {
                if(a.Size() == 3)
                {
                    if(a[1].AsValue() == "Deg")
                        return Deg(a[2].AsValue().Parse<Real>());
                    if(a[1].AsValue() == "Rad")
                        return Rad(a[2].AsValue().Parse<Real>());
                }
                throw ParamParsingError("ParamParser: unknown angle form");
            };

            auto &t = arr[i].AsArray();
            if(!t.Size())
                throw ParamParsingError("ParamParser: unknown transform form");

            auto &head = t[0].AsValue();

            if(head == "Scale")
            {
                if(t.Size() != 2)
                    throw ParamParsingError("ParamParser: unknown transform form");
                ret = ret * Transform::Scale(t[1].AsValue().Parse<Real>());
            }
            else if(head == "Translate")
            {
                if(t.Size() != 4)
                    throw ParamParsingError("ParamParser: unknown transform form");
                ret = ret * Transform::Translate(
                    t[1].AsValue().Parse<Real>(),
                    t[2].AsValue().Parse<Real>(),
                    t[3].AsValue().Parse<Real>());
            }
            else if(head == "RotateX")
            {
                ret = ret * Transform::RotateX(ExtractAngle(t));
            }
            else if(head == "RotateY")
            {
                ret = ret * Transform::RotateY(ExtractAngle(t));
            }
            else if(head == "RotateZ")
            {
                ret = ret * Transform::RotateZ(ExtractAngle(t));
            }
            else if(head == "Rotate")
            {
                if(t.Size() != 1 + 3 + 2)
                    throw ParamParsingError("ParamParser: unknown transform form");

                auto axis = Vec3(
                    t[1].AsValue().Parse<Real>(),
                    t[2].AsValue().Parse<Real>(),
                    t[3].AsValue().Parse<Real>());

                Rad rad;
                if(t[4].AsValue() == "Rad")
                    rad = Rad(t[5].AsValue().Parse<Real>());
                else if(t[4].AsValue() == "Deg")
                    rad = Deg(t[5].AsValue().Parse<Real>());
                else
                    throw ParamParsingError("ParamParser: unknown angle form");

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
